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

#ifndef TOOLBOX_NET_PROTOCOL_HPP
#define TOOLBOX_NET_PROTOCOL_HPP

#include <toolbox/Config.h>

#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

namespace toolbox {
inline namespace net {

struct DgramProtocol {
    friend bool operator==(DgramProtocol lhs, DgramProtocol rhs)
    {
        return lhs.family_ == rhs.family_ && lhs.protocol_ == rhs.protocol_;
    }
    friend bool operator!=(DgramProtocol lhs, DgramProtocol rhs) { return !(lhs == rhs); }

    constexpr explicit DgramProtocol(int family = AF_UNSPEC, int protocol = 0) noexcept
    : family_{family}
    , protocol_{protocol}
    {
    }
    static constexpr auto ip4() noexcept { return DgramProtocol{AF_INET, 0}; }
    static constexpr auto ip6() noexcept { return DgramProtocol{AF_INET6, 0}; }
    static constexpr auto udp4() noexcept { return DgramProtocol{AF_INET, IPPROTO_UDP}; }
    static constexpr auto udp6() noexcept { return DgramProtocol{AF_INET6, IPPROTO_UDP}; }
    static constexpr auto unix() noexcept { return DgramProtocol{AF_UNIX, 0}; }
    static constexpr auto unspec() noexcept { return DgramProtocol{AF_UNSPEC, 0}; }

    constexpr int family() const noexcept { return family_; }
    constexpr int type() const noexcept { return SOCK_DGRAM; }
    constexpr int protocol() const noexcept { return protocol_; }

  private:
    int family_, protocol_;
};

struct StreamProtocol {
    friend bool operator==(StreamProtocol lhs, StreamProtocol rhs)
    {
        return lhs.family_ == rhs.family_ && lhs.protocol_ == rhs.protocol_;
    }
    friend bool operator!=(StreamProtocol lhs, StreamProtocol rhs) { return !(lhs == rhs); }

    constexpr explicit StreamProtocol(int family = AF_UNSPEC, int protocol = 0) noexcept
    : family_{family}
    , protocol_{protocol}
    {
    }
    static constexpr auto ip4() noexcept { return StreamProtocol{AF_INET, 0}; }
    static constexpr auto ip6() noexcept { return StreamProtocol{AF_INET6, 0}; }
    static constexpr auto tcp4() noexcept { return StreamProtocol{AF_INET, IPPROTO_TCP}; }
    static constexpr auto tcp6() noexcept { return StreamProtocol{AF_INET6, IPPROTO_TCP}; }
    static constexpr auto unix() noexcept { return StreamProtocol{AF_UNIX, 0}; }
    static constexpr auto unspec() noexcept { return StreamProtocol{AF_UNSPEC, 0}; }

    constexpr int family() const noexcept { return family_; }
    constexpr int type() const noexcept { return SOCK_STREAM; }
    constexpr int protocol() const noexcept { return protocol_; }

  private:
    int family_, protocol_;
};

struct UdpProtocol {
    friend bool operator==(UdpProtocol lhs, UdpProtocol rhs) { return lhs.family_ == rhs.family_; }
    friend bool operator!=(UdpProtocol lhs, UdpProtocol rhs) { return lhs.family_ != rhs.family_; }

    static constexpr auto v4() noexcept { return UdpProtocol{AF_INET}; }
    static constexpr auto v6() noexcept { return UdpProtocol{AF_INET6}; }

    constexpr int family() const noexcept { return family_; }
    constexpr int type() const noexcept { return SOCK_DGRAM; }
    constexpr int protocol() const noexcept { return IPPROTO_UDP; }

  private:
    constexpr explicit UdpProtocol(int family) noexcept
    : family_{family}
    {
    }
    int family_;
};

struct TcpProtocol {
    friend bool operator==(TcpProtocol lhs, TcpProtocol rhs) { return lhs.family_ == rhs.family_; }
    friend bool operator!=(TcpProtocol lhs, TcpProtocol rhs) { return lhs.family_ != rhs.family_; }

    static constexpr auto v4() noexcept { return TcpProtocol{AF_INET}; }
    static constexpr auto v6() noexcept { return TcpProtocol{AF_INET6}; }

    constexpr int family() const noexcept { return family_; }
    constexpr int type() const noexcept { return SOCK_STREAM; }
    constexpr int protocol() const noexcept { return IPPROTO_TCP; }

  private:
    constexpr explicit TcpProtocol(int family) noexcept
    : family_{family}
    {
    }
    int family_;
};

struct UnixDgramProtocol {
    constexpr int family() const noexcept { return AF_UNIX; }
    constexpr int type() const noexcept { return SOCK_DGRAM; }
    constexpr int protocol() const noexcept { return 0; }
};

struct UnixStreamProtocol {
    constexpr int family() const noexcept { return AF_UNIX; }
    constexpr int type() const noexcept { return SOCK_STREAM; }
    constexpr int protocol() const noexcept { return 0; }
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_PROTOCOL_HPP
