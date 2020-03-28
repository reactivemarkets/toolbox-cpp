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

#ifndef TOOLBOX_NET_STREAMCONNECTOR_HPP
#define TOOLBOX_NET_STREAMCONNECTOR_HPP

#include <toolbox/io/Event.hpp>
#include <toolbox/io/Reactor.hpp>
#include <toolbox/net/StreamSock.hpp>

namespace toolbox {
inline namespace net {

template <typename DerivedT>
class StreamConnector {
  public:
    using Protocol = StreamProtocol;
    using Endpoint = StreamEndpoint;

    StreamConnector() = default;

    // Copy.
    StreamConnector(const StreamConnector&) = delete;
    StreamConnector& operator=(const StreamConnector&) = delete;

    // Move.
    StreamConnector(StreamConnector&&) = delete;
    StreamConnector& operator=(StreamConnector&&) = delete;

    /*
     * Returns true if connection was established synchronously or false if connection is pending
     * asynchronous completion.
     */
    bool connect(CyclTime now, Reactor& r, const Endpoint& ep)
    {
        StreamSockClnt sock{ep.protocol()};
        static_cast<DerivedT*>(this)->on_sock_prepare(now, sock);
        sock.set_non_block();
        if (sock.is_ip_family()) {
            set_tcp_no_delay(sock.get(), true);
        }

        std::error_code ec;
        sock.connect(ep, ec);
        if (ec) {
            if (ec != std::errc::operation_in_progress) {
                throw std::system_error{ec, "connect"};
            }
            sub_
                = r.subscribe(*sock, EpollIn | EpollOut, bind<&StreamConnector::on_io_event>(this));
            ep_ = ep;
            sock_ = std::move(sock);
            return false;
        }
        static_cast<DerivedT*>(this)->on_sock_connect(now, std::move(sock), ep);
        return true;
    }

  protected:
    ~StreamConnector() = default;

  private:
    void on_io_event(CyclTime now, int fd, unsigned events)
    {
        IoSock sock{std::move(sock_)};
        sub_.reset();
        try {
            const auto ec = sock.get_error();
            if (ec) {
                throw std::system_error{ec, "connect"};
            }
            static_cast<DerivedT*>(this)->on_sock_connect(now, std::move(sock), ep_);
        } catch (const std::exception& e) {
            static_cast<DerivedT*>(this)->on_sock_connect_error(now, e);
        }
    }

    Endpoint ep_;
    StreamSockClnt sock_;
    Reactor::Handle sub_;
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_STREAMCONNECTOR_HPP
