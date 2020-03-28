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

constexpr auto PingInterval = 100ms;

class EchoConn {

    // Automatically unlink when object is destroyed.
    using AutoUnlinkOption = boost::intrusive::link_mode<boost::intrusive::auto_unlink>;

  public:
    EchoConn(CyclTime now, Reactor& r, IoSock&& sock, const StreamEndpoint& ep)
    : sock_{move(sock)}
    , ep_{ep}
    {
        sub_ = r.subscribe(sock_.get(), EpollIn, bind<&EchoConn::on_input>(this));
        tmr_ = r.timer(now.mono_time(), PingInterval, Priority::Low,
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
                auto fn = [this](std::string_view line) {
                    ++count_;
                    // Echo bytes back to client.
                    TOOLBOX_INFO << "received: " << line;
                };
                buf_.consume(parse_line(buf_.str(), fn));
                if (count_ == 5) {
                    dispose(now);
                    return;
                }
            }
        } catch (const std::exception& e) {
            TOOLBOX_ERROR << "failed to read data: " << e.what();
            dispose(now);
        }
    }
    void on_timer(CyclTime now, Timer& tmr)
    {
        try {
            if (sock_.send("ping\n", 5, 0) < 5) {
                throw runtime_error{"partial write"};
            }
        } catch (const std::exception& e) {
            TOOLBOX_ERROR << "failed to write data: " << e.what();
            dispose(now);
        }
    }
    IoSock sock_;
    const StreamEndpoint ep_;
    Reactor::Handle sub_;
    Buffer buf_;
    Timer tmr_;
    int count_{0};
};

class EchoClnt : public StreamConnector<EchoClnt> {

    friend StreamConnector<EchoClnt>;
    using ConstantTimeSizeOption = boost::intrusive::constant_time_size<false>;
    using MemberHookOption = boost::intrusive::member_hook<EchoConn, decltype(EchoConn::list_hook),
                                                           &EchoConn::list_hook>;
    using ConnList = boost::intrusive::list<EchoConn, ConstantTimeSizeOption, MemberHookOption>;

  public:
    EchoClnt(CyclTime now, Reactor& reactor, Resolver& resolver, const string& uri)
    : reactor_{reactor}
    , resolver_{resolver}
    , uri_{uri}
    , ep_{parse_stream_endpoint(uri)}
    {
        // Immediate and then at 5s intervals.
        tmr_ = reactor_.timer(now.mono_time(), 5s, Priority::Low, bind<&EchoClnt::on_timer>(this));
    }
    ~EchoClnt()
    {
        const auto now = CyclTime::current();
        conn_list_.clear_and_dispose([now](auto* conn) { conn->dispose(now); });
    }

  private:
    void on_sock_prepare(CyclTime now, IoSock& sock)
    {
        if (sock.is_ip_family()) {
            // Set the number of SYN retransmits that TCP should send before aborting the attempt to
            // connect.
            set_tcp_syn_nt(sock.get(), 1);
        }
    }
    void on_sock_connect(CyclTime now, IoSock&& sock, const Endpoint& ep)
    {
        TOOLBOX_INFO << "connection opened: " << ep;
        inprogress_ = false;

        // High performance TCP servers could use a custom allocator.
        auto* const conn = new EchoConn{now, reactor_, move(sock), ep};
        conn_list_.push_back(*conn);
    }
    void on_sock_connect_error(CyclTime now, const std::exception& e)
    {
        TOOLBOX_ERROR << "failed to connect: " << e.what();
        aifuture_ = resolver_.resolve(uri_, SOCK_STREAM);
        inprogress_ = false;
    }
    void on_timer(CyclTime now, Timer& tmr)
    {
        if (!conn_list_.empty() || inprogress_) {
            return;
        }
        if (aifuture_.valid()) {
            if (!is_ready(aifuture_)) {
                TOOLBOX_INFO << "address pending";
                return;
            }
            try {
                ep_ = get_endpoint<Endpoint>(aifuture_);
            } catch (const std::exception& e) {
                TOOLBOX_ERROR << "failed to resolve address: " << e.what();
                aifuture_ = resolver_.resolve(uri_, SOCK_STREAM);
                return;
            }
            TOOLBOX_INFO << "address resolved: " << ep_;
        }
        TOOLBOX_INFO << "reconnecting";
        if (!connect(now, reactor_, ep_)) {
            inprogress_ = true;
        }
    }
    Reactor& reactor_;
    Resolver& resolver_;
    const string uri_;
    Timer tmr_;
    AddrInfoFuture aifuture_;
    Endpoint ep_;
    bool inprogress_{false};
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
        Resolver resolver;
        EchoClnt echo_clnt{start_time, reactor, resolver, "tcp4://127.0.0.1:7777"};

        // Start service threads.
        pthread_setname_np(pthread_self(), "main");
        ReactorRunner reactor_runner{reactor, "reactor"s};
        ResolverRunner resolver_runner{resolver, "resolver"s};

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
