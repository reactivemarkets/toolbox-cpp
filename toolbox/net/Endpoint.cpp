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

#include "Endpoint.hpp"

using namespace std;

namespace toolbox {
inline namespace net {
namespace {

pair<string, string> split_ip_addr(const string& addr, char delim)
{
    // Reverse find for compatibility with ipv6 address notation.
    const auto pos = addr.find_last_of(delim);
    string node, service;
    if (pos == string::npos) {
        node = addr;
    } else {
        node = addr.substr(0, pos);
        service = addr.substr(pos + 1);
    }
    // Remove square braces arround ipv6 address.
    if (node.size() >= 2 && node.front() == '[' && node.back() == ']') {
        node = node.substr(1, node.size() - 2);
    }
    return {node, service};
}

pair<string, string> split_uri(const string& uri)
{
    const auto pos = uri.find("://");
    string scheme, addr;
    if (pos == string::npos) {
        addr = uri;
    } else {
        scheme = uri.substr(0, pos);
        addr = uri.substr(pos + 3);
    }
    return {scheme, addr};
}
} // namespace

AddrInfoPtr parse_endpoint(const string& uri, int type)
{
    int family{-1}, protocol{0};
    const auto [scheme, addr] = split_uri(uri);
    if (scheme.empty()) {
        family = AF_UNSPEC;
    } else if (scheme == "ip4") {
        family = AF_INET;
    } else if (scheme == "ip6") {
        family = AF_INET6;
    } else if (scheme == "tcp4") {
        if (type == SOCK_STREAM) {
            family = AF_INET;
            protocol = IPPROTO_TCP;
        }
    } else if (scheme == "tcp6") {
        if (type == SOCK_STREAM) {
            family = AF_INET6;
            protocol = IPPROTO_TCP;
        }
    } else if (scheme == "udp4") {
        if (type == SOCK_DGRAM) {
            family = AF_INET;
            protocol = IPPROTO_UDP;
        }
    } else if (scheme == "udp6") {
        if (type == SOCK_DGRAM) {
            family = AF_INET6;
            protocol = IPPROTO_UDP;
        }
    } else if (scheme == "unix") {
        return get_unix_addrinfo(addr, type);
    }
    if (family < 0) {
        throw invalid_argument{"invalid uri: "s + uri};
    }
    auto [node, service] = split_ip_addr(addr, ':');
    return os::getaddrinfo(!node.empty() ? node.c_str() : nullptr,
                           !service.empty() ? service.c_str() : nullptr, family, type, protocol);
}

istream& operator>>(istream& is, DgramEndpoint& ep)
{
    string uri;
    if (is >> uri) {
        ep = parse_dgram_endpoint(uri);
    }
    return is;
}

istream& operator>>(istream& is, StreamEndpoint& ep)
{
    string uri;
    if (is >> uri) {
        ep = parse_stream_endpoint(uri);
    }
    return is;
}

ostream& operator<<(ostream& os, const DgramEndpoint& ep)
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
        scheme = "unix://";
    }
    return os << scheme << *ep.data();
}

ostream& operator<<(ostream& os, const StreamEndpoint& ep)
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
        scheme = "unix://";
    }
    return os << scheme << *ep.data();
}

} // namespace net
} // namespace toolbox

ostream& operator<<(ostream& os, const sockaddr_in& sa)
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &toolbox::remove_const(sa).sin_addr, buf, sizeof(buf));
    return os << buf << ':' << ntohs(sa.sin_port);
}

ostream& operator<<(ostream& os, const sockaddr_in6& sa)
{
    char buf[INET6_ADDRSTRLEN];
    inet_ntop(AF_INET6, &toolbox::remove_const(sa).sin6_addr, buf, sizeof(buf));
    return os << '[' << buf << "]:" << ntohs(sa.sin6_port);
}

ostream& operator<<(ostream& os, const sockaddr_un& sa)
{
    return os << sa.sun_path;
}

ostream& operator<<(ostream& os, const sockaddr& sa)
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

ostream& operator<<(ostream& os, const addrinfo& ai)
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
    return os << scheme << *ai.ai_addr;
}
