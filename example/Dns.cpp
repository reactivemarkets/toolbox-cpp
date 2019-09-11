// The Reactive C++ Toolbox.
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

#include <toolbox/io.hpp>
#include <toolbox/net.hpp>
#include <toolbox/sys.hpp>
#include <toolbox/util.hpp>

#include <fstream>
#include <iostream>

using namespace std;
using namespace toolbox;
using namespace toolbox::net;

class DnsRequestHandler {
  public:
    DnsRequestHandler(Reactor& reactor, IoSock& sock, const std::string& nameserver)
    : reactor_{reactor}
    , sock_{sock}
    , handle_{reactor.subscribe(*sock_, EventIn, bind<&DnsRequestHandler::on_request>(this))}
    , ep_{UdpProtocol::v4(), 63}
    , nameserver_{nameserver}
    {
    }

    void on_request(CyclTime now, int fd, unsigned events)
    {
        // TODO: Replace with protocol or allow bind to wrapped message types (MutableBuffer?)
        char size{0};
        os::recv(fd, &size, 1, 0);
        char hostname_buffer[64];
        os::recv(fd, hostname_buffer, size, 0);
        string_view hostname{hostname_buffer, static_cast<size_t>(size)};
        TOOLBOX_INFO << "Requesting " << hostname;
        auto udp_fd = send_dns_request(nameserver_, string{hostname}, DnsRequest::A);
        TOOLBOX_INFO << "Subscribing to response";
        response_
            = reactor_.subscribe(udp_fd, EventIn, bind<&DnsRequestHandler::on_response>(this));
    }

    void on_response(CyclTime now, int fd, unsigned events)
    {
        TOOLBOX_INFO << "Processing response";
        std::cout << get_dns_request(fd, ep_) << '\n';
        kill(getpid(), SIGTERM);
    }

  private:
    Reactor& reactor_;
    IoSock& sock_;
    Reactor::Handle handle_;
    Reactor::Handle response_;
    UdpEndpoint ep_;
    const std::string& nameserver_;
};

int main(int argc, char* argv[])
{
    std::ios::sync_with_stdio(false);
    int ret = 1;

    try {
        std::string nameserver;
        std::string hostname;

        Options opts{"Unit Test options [OPTIONS] [COMMAND]"};
        // clang-format off
        opts('s', Value{nameserver}, "ShortOption Description")
            ('h', "help", Help{}, "Help")
            (Value{hostname}.required(), "Hostname")
            ;
        // clang-format on

        opts.parse(argc, argv);

        if (nameserver.empty()) {
            std::ifstream resolv_conf{"/etc/resolv.conf"};
            DnsServers dns_servers{resolv_conf};
            nameserver = dns_servers.server();
        }

        EpollReactor reactor{1024};
        Resolver resolver;
        // Start service threads.
        pthread_setname_np(pthread_self(), "main");

        auto socks = socketpair(UnixStreamProtocol{});
        DnsRequestHandler h{reactor, socks.second, nameserver};

        ReactorRunner reactor_runner{reactor, "reactor"s};

        char request_buf[40];
        request_buf[0] = hostname.size();
        memcpy(&request_buf[1], hostname.data(), hostname.size());
        socks.first.send(request_buf, hostname.size() + 1, 0);

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
