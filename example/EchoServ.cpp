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

#include <toolbox/io.hpp>
#include <toolbox/net.hpp>
#include <toolbox/sys.hpp>
#include <toolbox/util.hpp>

#include <boost/intrusive/list.hpp>

using namespace std;
using namespace toolbox;

namespace {

constexpr auto IdleTimeout = 5s;

class EchoConn {

    // Automatically unlink when object is destroyed.
    using AutoUnlinkOption = boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

  public:
    template <typename EndpointT>
    EchoConn(CyclTime now, Reactor& r, IoSock&& sock, const EndpointT& ep)
    : reactor_{r}
    , sock_{move(sock)}
    , ep_{ep}
    {
        sub_ = r.subscribe(sock_.get(), EpollIn, bind<&EchoConn::on_input>(this));
        tmr_ = r.timer(now.mono_time() + IdleTimeout, Priority::Low,
                       bind<&EchoConn::on_timer>(this));
    }
    void dispose(CyclTime now) noexcept
    {
        TOOLBOX_INFO << "connection closed";
        delete this;
    }
    boost::intrusive::list_member_hook<AutoUnlinkOption> list_hook;

  private:
    ~EchoConn() = default;
    void on_input(CyclTime now, int fd, unsigned events)
    {
        try {
            if (events & (EpollIn | EpollHup)) {
                const auto size = os::read(fd, buf_.prepare(2944));
                if (size == 0) {
                    dispose(now);
                    return;
                }
                // Commit actual bytes read.
                buf_.commit(size);

                // Parse each buffered line.
                auto fn = [fd](std::string_view line) {
                    // Echo bytes back to client.
                    std::string buf{line};
                    buf += '\n';
                    if (os::write(fd, {buf.data(), buf.size()}) < buf.size()) {
                        throw runtime_error{"partial write"};
                    }
                };
                buf_.consume(parse_line(buf_.str(), fn));

                // Reset timer.
                tmr_.cancel();
                tmr_ = reactor_.timer(now.mono_time() + IdleTimeout, Priority::Low,
                                      bind<&EchoConn::on_timer>(this));
            }
        } catch (const std::exception& e) {
            TOOLBOX_ERROR << "exception: " << e.what();
            dispose(now);
        }
    }
    void on_timer(CyclTime now, Timer& tmr)
    {
        TOOLBOX_INFO << "timeout";
        dispose(now);
    }
    Reactor& reactor_;
    IoSock sock_;
    const StreamEndpoint ep_;
    Reactor::Handle sub_;
    Buffer buf_;
    Timer tmr_;
};

class EchoServ : public StreamAcceptor<EchoServ> {

    friend StreamAcceptor<EchoServ>;
    using ConstantTimeSizeOption = boost::intrusive::constant_time_size<false>;
    using MemberHookOption = boost::intrusive::member_hook<EchoConn, decltype(EchoConn::list_hook),
                                                           &EchoConn::list_hook>;
    using ConnList = boost::intrusive::list<EchoConn, ConstantTimeSizeOption, MemberHookOption>;

  public:
    EchoServ(CyclTime now, Reactor& r, const Endpoint& ep)
    : StreamAcceptor{r, ep}
    , reactor_{r}
    {
    }
    ~EchoServ()
    {
        const auto now = CyclTime::current();
        conn_list_.clear_and_dispose([now](auto* conn) { conn->dispose(now); });
    }

  private:
    void on_sock_prepare(CyclTime now, IoSock& sock) {}
    void on_sock_accept(CyclTime now, IoSock&& sock, const Endpoint& ep)
    {
        TOOLBOX_INFO << "connection opened: " << ep;

        // High performance TCP servers could use a custom allocator.
        auto* const conn = new EchoConn{now, reactor_, move(sock), ep};
        conn_list_.push_back(*conn);
    }
    Reactor& reactor_;
    // List of active connections.
    ConnList conn_list_;
};
} // namespace

int main(int argc, char* argv[])
{
    int ret = 1;
    try {

        const auto start_time = CyclTime::now();

        Reactor reactor{1024};
        const TcpEndpoint ep{TcpProtocol::v4(), 7777};
        EchoServ echo_serv{start_time, reactor, ep};

        // Start service threads.
        pthread_setname_np(pthread_self(), "main");
        ReactorRunner reactor_runner{reactor, "reactor"s};

        // Wait for termination.
        SigWait sig_wait;
        for (;;) {
            switch (const auto sig = sig_wait()) {
            case SIGHUP:
                TOOLBOX_INFO << "received SIGHUP";
                continue;
            case SIGINT:
                TOOLBOX_INFO << "received SIGINT";
                break;
            case SIGTERM:
                TOOLBOX_INFO << "received SIGTERM";
                break;
            default:
                TOOLBOX_INFO << "received signal: " << sig;
                continue;
            }
            break;
        }
        ret = 0;

    } catch (const std::exception& e) {
        TOOLBOX_ERROR << "exception: " << e.what();
    }
    return ret;
}
