// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2020 Reactive Markets Limited
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
#include <toolbox/io/Disposer.hpp>
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
, public BasicDisposer<BasicHttpConn<RequestT, AppT>>
, BasicHttpParser<BasicHttpConn<RequestT, AppT>> {

    friend class BasicDisposer<BasicHttpConn<RequestT, AppT>>;
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
    , reactor_{r}
    , sock_{std::move(sock)}
    , ep_{ep}
    , app_{app}
    {
        sub_ = r.subscribe(*sock_, EpollIn, bind<&BasicHttpConn::on_io_event>(this));
        schedule_timeout(now);
        app.on_http_connect(now, ep_);
    }

    // Copy.
    BasicHttpConn(const BasicHttpConn&) = delete;
    BasicHttpConn& operator=(const BasicHttpConn&) = delete;

    // Move.
    BasicHttpConn(BasicHttpConn&&) = delete;
    BasicHttpConn& operator=(BasicHttpConn&&) = delete;

    const Endpoint& endpoint() const noexcept { return ep_; }
    void clear() noexcept { req_.clear(); }
    boost::intrusive::list_member_hook<AutoUnlinkOption> list_hook;

  protected:
    void dispose_now(CyclTime now) noexcept
    {
        app_.on_http_disconnect(now, ep_); // noexcept
        // Best effort to drain any data still pending in the write buffer before the socket is
        // closed.
        if (!out_.empty()) {
            std::error_code ec;
            os::write(sock_.get(), out_.data(), ec); // noexcept
        }
        delete this;
    }

  private:
    ~BasicHttpConn() = default;
    bool on_message_begin(CyclTime now) noexcept
    {
        in_progress_ = true;
        req_.clear();
        return true;
    }
    bool on_url(CyclTime now, std::string_view sv) noexcept
    {
        bool ret{false};
        try {
            req_.append_url(sv);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
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
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
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
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
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
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
        }
        return ret;
    }
    bool on_message_end(CyclTime now) noexcept
    {
        bool ret{false};
        try {
            in_progress_ = false;
            req_.flush(); // May throw.
            app_.on_http_message(now, ep_, req_, os_);
            ret = true;
        } catch (const std::exception& e) {
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
        }
        return ret;
    }
    bool on_chunk_header(CyclTime now, std::size_t len) noexcept { return true; }
    bool on_chunk_end(CyclTime now) noexcept { return true; }
    void on_timeout_timer(CyclTime now, Timer& tmr)
    {
        auto lock = this->lock_this(now);
        app_.on_http_timeout(now, ep_);
        this->dispose(now);
    }
    void on_io_event(CyclTime now, int fd, unsigned events)
    {
        auto lock = this->lock_this(now);
        try {
            if (events & (EpollIn | EpollHup)) {
                if (!drain_input(now, fd)) {
                    this->dispose(now);
                    return;
                }
            }
            // Do not attempt to flush the output buffer if it is empty or if we are still waiting
            // for the socket to become writable.
            if (out_.empty() || (write_blocked_ && !(events & EpollOut))) {
                return;
            }
            flush_output(now);
        } catch (const HttpException&) {
            // Do not call on_http_error() here, because it will have already been called in one of
            // the noexcept parser callback functions.
        } catch (const std::exception& e) {
            app_.on_http_error(now, ep_, e, os_);
            this->dispose(now);
        }
    }
    bool drain_input(CyclTime now, int fd)
    {
        // Limit the number of reads to avoid starvation.
        for (int i{0}; i < 4; ++i) {
            std::error_code ec;
            const auto buf = in_.prepare(2944);
            const auto size = os::read(fd, buf, ec);
            if (ec) {
                // No data available in socket buffer.
                if (ec == std::errc::operation_would_block) {
                    break;
                }
                throw std::system_error{ec, "read"};
            }
            if (size == 0) {
                // N.B. the socket may still be writable if the peer has performed a shutdown on the
                // write side of the socket only.
                flush_input(now);
                return false;
            }
            // Commit actual bytes read.
            in_.commit(size);
            // Assume that the TCP stream has been drained if we read less than the requested
            // amount.
            if (static_cast<size_t>(size) < buffer_size(buf)) {
                break;
            }
        }
        flush_input(now);
        // Reset timer.
        schedule_timeout(now);
        return true;
    }
    void flush_input(CyclTime now) { in_.consume(parse(now, in_.data())); }
    void flush_output(CyclTime now)
    {
        // Attempt to flush buffered data.
        out_.consume(os::write(sock_.get(), out_.data()));
        if (out_.empty()) {
            if (!in_progress_ && !should_keep_alive()) {
                this->dispose(now);
                return;
            }
            if (write_blocked_) {
                // Restore read-only state after the buffer has been drained.
                sub_.set_events(EpollIn);
                write_blocked_ = false;
            }
        } else if (!write_blocked_) {
            // Set the state to read-write if the entire buffer could not be written.
            sub_.set_events(EpollIn | EpollOut);
            write_blocked_ = true;
        }
    }
    void schedule_timeout(CyclTime now)
    {
        const auto timeout = std::chrono::ceil<Seconds>(now.mono_time() + IdleTimeout);
        tmr_ = reactor_.timer(timeout, Priority::Low, bind<&BasicHttpConn::on_timeout_timer>(this));
    }

    Reactor& reactor_;
    IoSock sock_;
    Endpoint ep_;
    App& app_;
    Reactor::Handle sub_;
    Timer tmr_;
    Buffer in_, out_;
    Request req_;
    HttpStream os_{out_};
    bool in_progress_{false}, write_blocked_{false};
};

using HttpConn = BasicHttpConn<HttpRequest, HttpAppBase>;

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_CONN_HPP
