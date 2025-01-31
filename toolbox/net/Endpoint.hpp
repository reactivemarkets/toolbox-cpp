// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#ifndef TOOLBOX_NET_ENDPOINT_HPP
#define TOOLBOX_NET_ENDPOINT_HPP

#include <toolbox/net/Protocol.hpp>
#include <toolbox/net/Socket.hpp>
#include <toolbox/util/Concepts.hpp>
#include <toolbox/util/TypeTraits.hpp>

#include <boost/asio/generic/basic_endpoint.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>
#include <boost/asio/local/basic_endpoint.hpp>

namespace toolbox {
inline namespace net {

template <typename ProtocolT>
using BasicEndpoint = boost::asio::generic::basic_endpoint<ProtocolT>;

using DgramEndpoint = BasicEndpoint<DgramProtocol>;
using StreamEndpoint = BasicEndpoint<StreamProtocol>;

template <typename ProtocolT>
using IpEndpoint = boost::asio::ip::basic_endpoint<ProtocolT>;

using UdpEndpoint = IpEndpoint<UdpProtocol>;
using TcpEndpoint = IpEndpoint<TcpProtocol>;

template <typename ProtocolT>
using UnixEndpoint = boost::asio::local::basic_endpoint<ProtocolT>;

using UnixDgramEndpoint = UnixEndpoint<UnixDgramProtocol>;
using UnixStreamEndpoint = UnixEndpoint<UnixStreamProtocol>;

TOOLBOX_API AddrInfoPtr parse_endpoint(std::string_view uri, int type);

inline DgramEndpoint parse_dgram_endpoint(std::string_view uri)
{
    const auto ai = parse_endpoint(uri, SOCK_DGRAM);
    return {ai->ai_addr, ai->ai_addrlen, ai->ai_protocol};
}

inline StreamEndpoint parse_stream_endpoint(std::string_view uri)
{
    const auto ai = parse_endpoint(uri, SOCK_STREAM);
    return {ai->ai_addr, ai->ai_addrlen, ai->ai_protocol};
}

TOOLBOX_API std::istream& operator>>(std::istream& is, DgramEndpoint& ep);
TOOLBOX_API std::istream& operator>>(std::istream& is, StreamEndpoint& ep);

namespace detail {
template <typename StreamT, typename T>
    requires Streamable<StreamT>
StreamT& print_unix_endpoint(StreamT& os, const T& ep)
{
    constexpr const char* scheme = "unix://";
    // abstract unix socket's path starts with '\0' and not null-terminated
    const auto* path = reinterpret_cast<const sockaddr_un*>(ep.data())->sun_path;
    if (path[0] == '\0') {
        size_t size = ep.size() - sizeof(std::declval<sockaddr_un>().sun_family) - 1;
        os << scheme << '|' << std::string_view{path + 1, size};
        return os;
    }
    os << scheme << *ep.data();
    return os;
}
} // detail namespace

// Unfortunately, DgramEndpoint has an implicit converting constructor -- it accepts any type.
// Therefore, this overload becomes a valid candidate whenever a value of any type is being streamed
// into a stream -- resulting in ambiguous overload errors. DgramEndpoint is defined in boost::asio
// so the impl can't be modified. Therefore, a templated type is used to constrain this overload to
// DgramEndpoint only.
template <typename StreamT, class T>
    requires Streamable<StreamT> && std::same_as<T, DgramEndpoint>
StreamT& operator<<(StreamT& os, const T& ep)
{
    const char* scheme = "";
    const auto p = ep.protocol();
    if (p.family() == AF_INET) {
        if (p.protocol() == IPPROTO_UDP) {
            scheme = "udp4://";
        } else {
            scheme = "ip4://";
        }
    } else if (p.family() == AF_INET6) {
        if (p.protocol() == IPPROTO_UDP) {
            scheme = "udp6://";
        } else {
            scheme = "ip6://";
        }
    } else if (p.family() == AF_UNIX) {
        return detail::print_unix_endpoint(os, ep);
    }
    os << scheme << *ep.data();
    return os;
}

// Unfortunately, StreamEndpoint has an implicit converting constructor -- it accepts any type.
// Therefore, this overload becomes a valid candidate whenever a value of any type is being streamed
// into a stream -- resulting in ambiguous overload errors. StreamEndpoint is defined in boost::asio
// so the impl can't be modified. Therefore, a templated type is used to constrain this overload to
// StreamEndpoint only.
template <typename StreamT, class T>
    requires Streamable<StreamT> && std::same_as<T, StreamEndpoint>
StreamT& operator<<(StreamT& os, const T& ep)
{
    const char* scheme = "";
    const auto p = ep.protocol();
    if (p.family() == AF_INET) {
        if (p.protocol() == IPPROTO_TCP) {
            scheme = "tcp4://";
        } else {
            scheme = "ip4://";
        }
    } else if (p.family() == AF_INET6) {
        if (p.protocol() == IPPROTO_TCP) {
            scheme = "tcp6://";
        } else {
            scheme = "ip6://";
        }
    } else if (p.family() == AF_UNIX) {
        return detail::print_unix_endpoint(os, ep);
    }
    os << scheme << *ep.data();
    return os;
}

} // namespace net
inline namespace util {

template <>
struct TypeTraits<DgramEndpoint> {
    static auto from_string(std::string_view sv) { return parse_dgram_endpoint(std::string{sv}); }
    static auto from_string(const std::string& s) { return parse_dgram_endpoint(s); }
};
template <>
struct TypeTraits<StreamEndpoint> {
    static auto from_string(std::string_view sv) { return parse_stream_endpoint(std::string{sv}); }
    static auto from_string(const std::string& s) { return parse_stream_endpoint(s); }
};

} // namespace util
} // namespace toolbox

template <typename StreamT>
    requires toolbox::util::Streamable<StreamT>
StreamT& operator<<(StreamT& os, const sockaddr_in& sa)
{
    // biggest possible str: 255.255.255.255:
    char buf[16];
    char* p = buf;

    auto write_u8 = [&p](std::uint8_t v) {
        char rd = '0' + (v % 10u);
        if (v >= 100u) {
            char ld = '0' + ((v / 100u) % 10u);
            char md = '0' + ((v / 10u) % 10u);
            *p++ = ld;
            *p++ = md;
            *p++ = rd;
        } else if (v >= 10u) {
            char ld = '0' + ((v / 10u) % 10u);
            *p++ = ld;
            *p++ = rd;
        } else {
            *p++ = rd;
        }
    };

    // ip address in sockaddr_in is in network order (i.e. big endian)
    uint32_t addr = sa.sin_addr.s_addr;
    auto* ipv4 = std::bit_cast<unsigned char*>(&addr);

    write_u8(ipv4[0]);
    *p++ = '.';
    write_u8(ipv4[1]);
    *p++ = '.';
    write_u8(ipv4[2]);
    *p++ = '.';
    write_u8(ipv4[3]);
    *p++ = ':';

    os << std::string_view(buf, p) << ntohs(sa.sin_port);
    return os;
}

template <typename StreamT>
    requires toolbox::util::Streamable<StreamT>
StreamT& operator<<(StreamT& os, const sockaddr_in6& sa)
{
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &toolbox::remove_const(sa).sin6_addr, buf, sizeof(buf));
    os << '[' << buf << "]:" << ntohs(sa.sin6_port);
    return os;
}

template <typename StreamT>
    requires toolbox::util::Streamable<StreamT>
StreamT& operator<<(StreamT& os, const sockaddr_un& sa)
{
    os << sa.sun_path;
    return os;
}

template <typename StreamT>
    requires toolbox::util::Streamable<StreamT>
StreamT& operator<<(StreamT& os, const sockaddr& sa)
{
    if (sa.sa_family == AF_INET) {
        os << reinterpret_cast<const sockaddr_in&>(sa);
    } else if (sa.sa_family == AF_INET6) {
        os << reinterpret_cast<const sockaddr_in6&>(sa);
    } else if (sa.sa_family == AF_UNIX) {
        os << reinterpret_cast<const sockaddr_un&>(sa);
    } else {
        os << "<sockaddr>";
    }
    return os;
}

template <typename StreamT>
    requires toolbox::util::Streamable<StreamT>
StreamT& operator<<(StreamT& os, const addrinfo& ai)
{
    const char* scheme = "";
    if (ai.ai_family == AF_INET) {
        if (ai.ai_protocol == IPPROTO_TCP) {
            scheme = "tcp4://";
        } else if (ai.ai_protocol == IPPROTO_UDP) {
            scheme = "udp4://";
        } else {
            scheme = "ip4://";
        }
    } else if (ai.ai_family == AF_INET6) {
        if (ai.ai_protocol == IPPROTO_TCP) {
            scheme = "tcp6://";
        } else if (ai.ai_protocol == IPPROTO_UDP) {
            scheme = "udp6://";
        } else {
            scheme = "ip6://";
        }
    } else if (ai.ai_family == AF_UNIX) {
        scheme = "unix://";
    }
    os << scheme << *ai.ai_addr;
    return os;
}

#endif // TOOLBOX_NET_ENDPOINT_HPP
