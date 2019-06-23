// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef TOOLBOX_HTTP_CONN_HPP
#define TOOLBOX_HTTP_CONN_HPP

#include <toolbox/http/Parser.hpp>
#include <toolbox/http/Request.hpp>
#include <toolbox/http/Stream.hpp>
#include <toolbox/io/Event.hpp>
#include <toolbox/io/Reactor.hpp>
#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IoSock.hpp>
#include <toolbox/util/MemAlloc.hpp>

#include <boost/intrusive/list.hpp>

namespace toolbox {
inline namespace http {
class HttpAppBase;

template <typename RequestT, typename AppT>
class BasicHttpConn
: public MemAlloc
, BasicHttpParser<BasicHttpConn<RequestT, AppT>> {

    friend class BasicHttpParser<BasicHttpConn<RequestT, AppT>>;

    using Request = RequestT;
    using App = AppT;
    // Automatically unlink when object is destroyed.
    using AutoUnlinkOption = boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

    static constexpr auto IdleTimeout = 5s;

    using BasicHttpParser<BasicHttpConn<RequestT, AppT>>::method;
    using BasicHttpParser<BasicHttpConn<RequestT, AppT>>::parse;
    using BasicHttpParser<BasicHttpConn<RequestT, AppT>>::should_keep_alive;

  public:
    using Protocol = StreamProtocol;
    using Endpoint = StreamEndpoint;

    BasicHttpConn(CyclTime now, Reactor& r, IoSock&& sock, const Endpoint& ep, App& app)
    : BasicHttpParser<BasicHttpConn<RequestT, AppT>>{HttpType::Request}
    , reactor_(r)
    , sock_{std::move(sock)}
    , ep_{ep}
    , app_(app)
    {
        sub_ = r.subscribe(*sock_, EventIn, bind<&BasicHttpConn::on_io_event>(this));
        tmr_ = r.timer(now.mono_time() + IdleTimeout, Priority::Low,
                       bind<&BasicHttpConn::on_timer>(this));
        app.on_connect(now, ep_);
    }

    // Copy.
    BasicHttpConn(const BasicHttpConn&) = delete;
    BasicHttpConn& operator=(const BasicHttpConn&) = delete;

    // Move.
    BasicHttpConn(BasicHttpConn&&) = delete;
    BasicHttpConn& operator=(BasicHttpConn&&) = delete;

    const Endpoint& endpoint() const noexcept { return ep_; }
    void clear() noexcept { req_.clear(); }
    void dispose(CyclTime now) noexcept
    {
        app_.on_disconnect(now, ep_);
        delete this;
    }

    boost::intrusive::list_member_hook<AutoUnlinkOption> list_hook;

  private:
    ~BasicHttpConn() = default;
    bool on_message_begin(CyclTime now) noexcept
    {
        ++pending_;
        return true;
    }
    bool on_url(CyclTime now, std::string_view sv) noexcept
    {
        bool ret{false};
        try {
            req_.append_url(sv);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
        return ret;
    }
    bool on_status(CyclTime now, std::string_view sv) noexcept
    {
        // Only supported for HTTP responses.
        return false;
    }
    bool on_header_field(CyclTime now, std::string_view sv, First first) noexcept
    {
        bool ret{false};
        try {
            req_.append_header_field(sv, first);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
        return ret;
    }
    bool on_header_value(CyclTime now, std::string_view sv, First first) noexcept
    {
        bool ret{false};
        try {
            req_.append_header_value(sv, first);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
        return ret;
    }
    bool on_headers_end(CyclTime now) noexcept
    {
        req_.set_method(method());
        return true;
    }
    bool on_body(CyclTime now, std::string_view sv) noexcept
    {
        bool ret{false};
        try {
            req_.append_body(sv);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
        return ret;
    }
    bool on_message_end(CyclTime now) noexcept
    {
        bool ret{false};
        try {
            --pending_;
            req_.flush(); // May throw.

            const auto was_empty = out_.empty();
            app_.on_message(now, ep_, req_, os_);

            if (was_empty) {
                // May throw.
                sub_.set_events(EventIn | EventOut);
            }
            ret = true;
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
        req_.clear();
        return ret;
    }
    bool on_chunk_header(CyclTime now, std::size_t len) noexcept { return true; }
    bool on_chunk_end(CyclTime now) noexcept { return true; }
    void on_io_event(CyclTime now, int fd, unsigned events)
    {
        try {
            if (events & EventOut) {
                out_.consume(os::write(fd, out_.data()));
                if (out_.empty()) {
                    if (!dispose_ && (pending_ || should_keep_alive())) {
                        // May throw.
                        sub_.set_events(EventIn);
                    } else {
                        dispose(now);
                    }
                }
            }
            if (events & (EventIn | EventHup)) {
                const auto size = os::read(fd, in_.prepare(2048));
                if (size > 0) {
                    // Commit actual bytes read.
                    in_.commit(size);
                    in_.consume(parse(now, in_.data()));

                    // Reset timer.
                    tmr_.cancel();
                    tmr_ = reactor_.timer(now.mono_time() + IdleTimeout, Priority::Low,
                                          bind<&BasicHttpConn::on_timer>(this));
                } else {
                    dispose(now);
                }
            }
        } catch (const HttpException&) {
            // Do not call on_error() here, because it will have already been called in one of the
            // noexcept parser callback functions.
        } catch (const std::exception& e) {
            app_.on_error(now, ep_, e, os_);
            flush_and_dispose(now);
        }
    }
    void on_timer(CyclTime now, Timer& tmr)
    {
        app_.on_timeout(now, ep_);
        dispose(now);
    }
    void flush_and_dispose(CyclTime now) noexcept
    {
        if (out_.empty()) {
            dispose(now);
        } else {
            std::error_code ec;
            sub_.set_events(EventIn | EventOut, ec);
            if (ec) {
                // Failed to set events.
                dispose(now);
            } else {
                dispose_ = true;
            }
        }
    }

    Reactor& reactor_;
    IoSock sock_;
    Endpoint ep_;
    App& app_;
    Reactor::Handle sub_;
    Timer tmr_;
    bool dispose_{false};
    int pending_{0};
    Buffer in_, out_;
    Request req_;
    HttpStream os_{out_};
};

using HttpConn = BasicHttpConn<HttpRequest, HttpAppBase>;

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_CONN_HPP
