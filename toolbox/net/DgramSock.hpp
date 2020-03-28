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

#ifndef TOOLBOX_NET_DGRAMSOCK_HPP
#define TOOLBOX_NET_DGRAMSOCK_HPP

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IoSock.hpp>

namespace toolbox {
inline namespace net {

/// Connectionless Datagram Socket. All state is in base class, so object can be sliced.
struct DgramSock : IoSock {
    using Protocol = DgramProtocol;
    using Endpoint = DgramEndpoint;

    using IoSock::IoSock;

    DgramSock(Protocol protocol, std::error_code& ec) noexcept
    : IoSock{os::socket(protocol, ec), protocol.family()}
    {
    }
    explicit DgramSock(Protocol protocol)
    : IoSock{os::socket(protocol), protocol.family()}
    {
    }
    DgramSock() noexcept = default;

    // Logically const.
    void get_sock_name(Endpoint& ep, std::error_code& ec) noexcept
    {
        os::getsockname(get(), ep, ec);
    }
    void get_sock_name(Endpoint& ep) { os::getsockname(get(), ep); }
    void bind(const Endpoint& ep, std::error_code& ec) noexcept { os::bind(get(), ep, ec); }
    void bind(const Endpoint& ep) { os::bind(get(), ep); }
    void connect(const Endpoint& ep, std::error_code& ec) noexcept
    {
        return os::connect(get(), ep, ec);
    }
    void connect(const Endpoint& ep) { return os::connect(get(), ep); }

    ssize_t recvfrom(void* buf, std::size_t len, int flags, Endpoint& ep,
                     std::error_code& ec) noexcept
    {
        return os::recvfrom(get(), buf, len, flags, ep, ec);
    }
    std::size_t recvfrom(void* buf, std::size_t len, int flags, Endpoint& ep)
    {
        return os::recvfrom(get(), buf, len, flags, ep);
    }

    ssize_t recvfrom(MutableBuffer buf, int flags, Endpoint& ep, std::error_code& ec) noexcept
    {
        return os::recvfrom(get(), buf, flags, ep, ec);
    }
    std::size_t recvfrom(MutableBuffer buf, int flags, Endpoint& ep)
    {
        return os::recvfrom(get(), buf, flags, ep);
    }

    ssize_t sendto(const void* buf, std::size_t len, int flags, const Endpoint& ep,
                   std::error_code& ec) noexcept
    {
        return os::sendto(get(), buf, len, flags, ep, ec);
    }
    std::size_t sendto(const void* buf, std::size_t len, int flags, const Endpoint& ep)
    {
        return os::sendto(get(), buf, len, flags, ep);
    }

    ssize_t sendto(ConstBuffer buf, int flags, const Endpoint& ep, std::error_code& ec) noexcept
    {
        return os::sendto(get(), buf, flags, ep, ec);
    }
    std::size_t sendto(ConstBuffer buf, int flags, const Endpoint& ep)
    {
        return os::sendto(get(), buf, flags, ep);
    }
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_DGRAMSOCK_HPP
