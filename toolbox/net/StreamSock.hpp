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

#ifndef TOOLBOX_NET_STREAMSOCK_HPP
#define TOOLBOX_NET_STREAMSOCK_HPP

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IoSock.hpp>

namespace toolbox {
inline namespace net {

/// Passive Server Stream Socket. All state is in base class, so object can be sliced.
struct StreamSockServ : Sock {
    using Protocol = StreamProtocol;
    using Endpoint = StreamEndpoint;

    using Sock::Sock;

    StreamSockServ(Protocol protocol, std::error_code& ec) noexcept
    : Sock{os::socket(protocol, ec), protocol.family()}
    {
    }
    explicit StreamSockServ(Protocol protocol)
    : Sock{os::socket(protocol), protocol.family()}
    {
    }
    StreamSockServ() noexcept = default;

    // Logically const.
    void get_sock_name(Endpoint& ep, std::error_code& ec) noexcept
    {
        os::getsockname(get(), ep, ec);
    }
    void get_sock_name(Endpoint& ep) { os::getsockname(get(), ep); }
    void bind(const Endpoint& ep, std::error_code& ec) noexcept { os::bind(get(), ep, ec); }
    void bind(const Endpoint& ep) { os::bind(get(), ep); }

    void listen(int backlog, std::error_code& ec) noexcept { os::listen(get(), backlog, ec); }
    void listen(int backlog) { os::listen(get(), backlog); }

    IoSock accept(Endpoint& ep, std::error_code& ec) noexcept
    {
        return IoSock{os::accept(get(), ep, ec), family()};
    }
    IoSock accept(Endpoint& ep) { return IoSock{os::accept(get(), ep), family()}; }
};

/// Active Client Stream Socket. All state is in base class, so object can be sliced.
struct StreamSockClnt : IoSock {
    using Protocol = StreamProtocol;
    using Endpoint = StreamEndpoint;

    using IoSock::IoSock;

    StreamSockClnt(Protocol protocol, std::error_code& ec) noexcept
    : IoSock{os::socket(protocol, ec), protocol.family()}
    {
    }
    explicit StreamSockClnt(Protocol protocol)
    : IoSock{os::socket(protocol), protocol.family()}
    {
    }
    StreamSockClnt() noexcept = default;

    // Logically const.
    void get_sock_name(Endpoint& ep, std::error_code& ec) noexcept
    {
        os::getsockname(get(), ep, ec);
    }
    void get_sock_name(Endpoint& ep) { os::getsockname(get(), ep); }
    void connect(const Endpoint& ep, std::error_code& ec) noexcept
    {
        return os::connect(get(), ep, ec);
    }
    void connect(const Endpoint& ep) { return os::connect(get(), ep); }
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_STREAMSOCK_HPP
