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

#ifndef TOOLBOX_NET_MCASTSOCK_HPP
#define TOOLBOX_NET_MCASTSOCK_HPP

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IoSock.hpp>
#include <toolbox/net/IpAddr.hpp>

namespace toolbox {
inline namespace net {

struct TOOLBOX_API IpMcastGroup {
    IpMcastGroup(const IpAddr& addr, unsigned ifindex = 0) noexcept;

    IpMcastGroup(const IpAddr& addr, const char* ifname, std::error_code& ec) noexcept
    : IpMcastGroup(addr, os::if_nametoindex(ifname, ec))
    {
    }
    IpMcastGroup(const IpAddr& addr, const char* ifname)
    : IpMcastGroup(addr, os::if_nametoindex(ifname))
    {
    }

    int family;
    union {
        ip_mreqn ipv4;
        ipv6_mreq ipv6;
    };
};

/// Join a multicast group.
inline void join_group(int sockfd, const IpMcastGroup& group, std::error_code& ec) noexcept
{
    if (group.family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group.ipv6, sizeof(group.ipv6), ec);
    } else {
        assert(group.family == AF_INET);
        os::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group.ipv4, sizeof(group.ipv4), ec);
    }
}

/// Join a multicast group.
inline void join_group(int sockfd, const IpMcastGroup& group)
{
    if (group.family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_JOIN_GROUP, &group.ipv6, sizeof(group.ipv6));
    } else {
        assert(group.family == AF_INET);
        os::setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &group.ipv4, sizeof(group.ipv4));
    }
}

/// Join a multicast group.
inline void join_group(int sockfd, const IpAddr& addr, unsigned ifindex,
                       std::error_code& ec) noexcept
{
    join_group(sockfd, IpMcastGroup{addr, ifindex}, ec);
}

/// Join a multicast group.
inline void join_group(int sockfd, const IpAddr& addr, unsigned ifindex)
{
    join_group(sockfd, IpMcastGroup{addr, ifindex});
}

/// Join a multicast group. The system will choose an appropriate interface if ifname is null.
inline void join_group(int sockfd, const IpAddr& addr, const char* ifname,
                       std::error_code& ec) noexcept
{
    join_group(sockfd, IpMcastGroup{addr, ifname}, ec);
}

/// Join a multicast group. The system will choose an appropriate interface if ifname is null.
inline void join_group(int sockfd, const IpAddr& addr, const char* ifname)
{
    join_group(sockfd, IpMcastGroup{addr, ifname});
}

/// Leave a multicast group.
inline void leave_group(int sockfd, const IpMcastGroup& group, std::error_code& ec) noexcept
{
    if (group.family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &group.ipv6, sizeof(group.ipv6), ec);
    } else {
        assert(group.family == AF_INET);
        os::setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group.ipv4, sizeof(group.ipv4), ec);
    }
}

/// Leave a multicast group.
inline void leave_group(int sockfd, const IpMcastGroup& group)
{
    if (group.family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &group.ipv6, sizeof(group.ipv6));
    } else {
        assert(group.family == AF_INET);
        os::setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP, &group.ipv4, sizeof(group.ipv4));
    }
}

/// Leave a multicast group.
inline void leave_group(int sockfd, const IpAddr& addr, unsigned ifindex,
                        std::error_code& ec) noexcept
{
    leave_group(sockfd, IpMcastGroup{addr, ifindex}, ec);
}

/// Leave a multicast group.
inline void leave_group(int sockfd, const IpAddr& addr, unsigned ifindex)
{
    leave_group(sockfd, IpMcastGroup{addr, ifindex});
}

/// Leave a multicast group. The system will leave on the first matching interface if ifname is null.
inline void leave_group(int sockfd, const IpAddr& addr, const char* ifname,
                        std::error_code& ec) noexcept
{
    leave_group(sockfd, IpMcastGroup{addr, ifname}, ec);
}

/// Leave a multicast group. The system will leave on the first matching interface if ifname is null.
inline void leave_group(int sockfd, const IpAddr& addr, const char* ifname)
{
    leave_group(sockfd, IpMcastGroup{addr, ifname});
}

inline void set_ip_mcast_if(int sockfd, int family, unsigned ifindex, std::error_code& ec) noexcept
{
    if (family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex), ec);
    } else {
        assert(family == AF_INET);
        ip_mreqn mreqn{};
        mreqn.imr_ifindex = ifindex;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreqn, sizeof(mreqn), ec);
    }
}

inline void set_ip_mcast_if(int sockfd, int family, unsigned ifindex)
{
    if (family == AF_INET6) {
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &ifindex, sizeof(ifindex));
    } else {
        assert(family == AF_INET);
        ip_mreqn mreqn{};
        mreqn.imr_ifindex = ifindex;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_IF, &mreqn, sizeof(mreqn));
    }
}

inline void set_ip_mcast_if(int sockfd, int family, const char* ifname,
                            std::error_code& ec) noexcept
{
    const auto ifindex = os::if_nametoindex(ifname, ec);
    if (!ec) {
        set_ip_mcast_if(sockfd, family, ifindex, ec);
    }
}

inline void set_ip_mcast_if(int sockfd, int family, const char* ifname)
{
    set_ip_mcast_if(sockfd, family, os::if_nametoindex(ifname));
}

/// Determines whether sent multicast packets should be looped back to the local sockets.
inline void set_ip_mcast_loop(int sockfd, int family, bool enabled, std::error_code& ec) noexcept
{
    if (family == AF_INET6) {
        const unsigned optval{enabled ? 1U : 0U};
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &optval, sizeof(optval), ec);
    } else {
        assert(family == AF_INET);
        const unsigned char optval = enabled ? 1 : 0;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &optval, sizeof(optval), ec);
    }
}

/// Determines whether sent multicast packets should be looped back to the local sockets.
inline void set_ip_mcast_loop(int sockfd, int family, bool enabled)
{
    if (family == AF_INET6) {
        const unsigned optval{enabled ? 1U : 0U};
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &optval, sizeof(optval));
    } else {
        assert(family == AF_INET);
        const unsigned char optval = enabled ? 1 : 0;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_LOOP, &optval, sizeof(optval));
    }
}

/// Set or read the time-to-live value of outgoing multicast packets for this socket.
inline void set_ip_mcast_ttl(int sockfd, int family, int ttl, std::error_code& ec) noexcept
{
    if (family == AF_INET6) {
        const int optval{ttl};
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &optval, sizeof(optval), ec);
    } else {
        assert(family == AF_INET);
        const unsigned char optval = ttl;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval), ec);
    }
}

/// Set or read the time-to-live value of outgoing multicast packets for this socket.
inline void set_ip_mcast_ttl(int sockfd, int family, int ttl)
{
    if (family == AF_INET6) {
        const int optval{ttl};
        os::setsockopt(sockfd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &optval, sizeof(optval));
    } else {
        assert(family == AF_INET);
        const unsigned char optval = ttl;
        os::setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL, &optval, sizeof(optval));
    }
}

/// Connectionless Datagram Socket. All state is in base class, so object can be sliced.
struct McastSock : IoSock {
    using Protocol = UdpProtocol;
    using Endpoint = UdpEndpoint;

    using IoSock::IoSock;

    McastSock(Protocol protocol, std::error_code& ec) noexcept
    : IoSock{os::socket(protocol, ec), protocol.family()}
    {
    }
    explicit McastSock(Protocol protocol)
    : IoSock{os::socket(protocol), protocol.family()}
    {
    }
    McastSock() noexcept = default;

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

    void join_group(const IpAddr& addr, unsigned ifindex, std::error_code& ec) noexcept
    {
        return toolbox::join_group(get(), addr, ifindex, ec);
    }
    void join_group(const IpAddr& addr, unsigned ifindex)
    {
        return toolbox::join_group(get(), addr, ifindex);
    }

    void join_group(const IpAddr& addr, const char* ifname, std::error_code& ec) noexcept
    {
        return toolbox::join_group(get(), addr, ifname, ec);
    }
    void join_group(const IpAddr& addr, const char* ifname)
    {
        return toolbox::join_group(get(), addr, ifname);
    }

    void leave_group(const IpAddr& addr, unsigned ifindex, std::error_code& ec) noexcept
    {
        return toolbox::leave_group(get(), addr, ifindex, ec);
    }
    void leave_group(const IpAddr& addr, unsigned ifindex)
    {
        return toolbox::leave_group(get(), addr, ifindex);
    }

    void leave_group(const IpAddr& addr, const char* ifname, std::error_code& ec) noexcept
    {
        return toolbox::leave_group(get(), addr, ifname, ec);
    }
    void leave_group(const IpAddr& addr, const char* ifname)
    {
        return toolbox::leave_group(get(), addr, ifname);
    }

    void set_ip_mcast_if(const char* ifname, std::error_code& ec) noexcept
    {
        return toolbox::set_ip_mcast_if(get(), family(), ifname, ec);
    }
    void set_ip_mcast_if(const char* ifname)
    {
        return toolbox::set_ip_mcast_if(get(), family(), ifname);
    }

    void set_ip_mcast_loop(bool enabled, std::error_code& ec) noexcept
    {
        return toolbox::set_ip_mcast_loop(get(), family(), enabled, ec);
    }
    void set_ip_mcast_loop(bool enabled)
    {
        return toolbox::set_ip_mcast_loop(get(), family(), enabled);
    }

    void set_ip_mcast_ttl(int ttl, std::error_code& ec) noexcept
    {
        return toolbox::set_ip_mcast_ttl(get(), family(), ttl, ec);
    }
    void set_ip_mcast_ttl(int ttl) { return toolbox::set_ip_mcast_ttl(get(), family(), ttl); }
};

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_MCASTSOCK_HPP
