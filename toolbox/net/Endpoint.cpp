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

#include "Endpoint.hpp"

#include <toolbox/util/String.hpp>

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
    // Remove square braces around ipv6 address.
    if (node.size() >= 2 && node.front() == '[' && node.back() == ']') {
        node = node.substr(1, node.size() - 2);
    }
    return {node, service};
}

pair<string, string> split_uri(string_view uri)
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

AddrInfoPtr parse_endpoint(string_view uri, int type)
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
        throw invalid_argument{make_string("invalid uri: ", uri)};
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

} // namespace net
} // namespace toolbox
