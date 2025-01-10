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
#include <toolbox/util/Random.hpp>
#include <toolbox/util/Stream.hpp>

#include <boost/test/unit_test.hpp>

#include <vector>

using namespace std;
using namespace toolbox;

namespace {
std::vector<sockaddr_in> generate_random_ipv4_addresses(size_t N) {
    std::vector<sockaddr_in> ret;
    ret.reserve(N);

    for (size_t i = 0; i < N; i++) {
        sockaddr_in s;
        s.sin_family = AF_INET;
        s.sin_port = htons(randint<in_port_t>(0, std::numeric_limits<in_port_t>::max()));
        s.sin_addr = in_addr{randint<uint32_t>(0, std::numeric_limits<uint32_t>::max())};
        ret.push_back(s);
    }

    return ret;
}

ostream& write_ipv4_libc(ostream& os, const sockaddr_in& sa)
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &toolbox::remove_const(sa).sin_addr, buf, sizeof(buf));
    return os << buf << ':' << ntohs(sa.sin_port);
}

util::OStream<32> ipv4_os{nullptr};

} // namespace

BOOST_AUTO_TEST_SUITE(EndpointSuite)

BOOST_AUTO_TEST_CASE(ParseDgramUnspec4Case)
{
    const auto uri = "192.168.1.3:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_UDP);
    BOOST_CHECK_EQUAL(to_string(ep), "udp4://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUnspec6Case)
{
    const auto uri = "[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET6);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_UDP);
    BOOST_CHECK_EQUAL(to_string(ep), "udp6://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUdp4Case)
{
    const auto uri = "udp4://192.168.1.3:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_UDP);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUdp6Case)
{
    const auto uri = "udp6://[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET6);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_UDP);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUnixCase)
{
    const auto uri = "unix:///tmp/foo.sock"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_UNIX);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), 0);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUnixAbstractCase)
{
    const auto uri = "unix://|12345"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_UNIX);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_DGRAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), 0);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnspec4Case)
{
    const auto uri = "192.168.1.3:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_TCP);
    BOOST_CHECK_EQUAL(to_string(ep), "tcp4://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnspec6Case)
{
    const auto uri = "[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET6);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_TCP);
    BOOST_CHECK_EQUAL(to_string(ep), "tcp6://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamTcp4Case)
{
    const auto uri = "tcp4://192.168.1.3:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_TCP);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamTcp6Case)
{
    const auto uri = "tcp6://[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET6);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_TCP);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnixCase)
{
    const auto uri = "unix:///tmp/foo.sock"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_UNIX);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), 0);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnixAbstractCase)
{
    const auto uri = "unix://|12345"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_UNIX);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), 0);
    BOOST_CHECK_EQUAL(to_string(ep), uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamBindCase)
{
    const auto ep = parse_stream_endpoint("tcp4://:80");
    BOOST_CHECK_EQUAL(ep.protocol().family(), AF_INET);
    BOOST_CHECK_EQUAL(ep.protocol().type(), SOCK_STREAM);
    BOOST_CHECK_EQUAL(ep.protocol().protocol(), IPPROTO_TCP);
    BOOST_CHECK_EQUAL(to_string(ep), "tcp4://0.0.0.0:80");
}

BOOST_AUTO_TEST_CASE(IPv4Formatting)
{
    ipv4_os.set_storage(ipv4_os.make_storage());

    const auto rand_ips = generate_random_ipv4_addresses(131'072);
    for (const auto& ip : rand_ips) {
        ipv4_os.reset();
        ipv4_os << ip;
        std::string our_str {ipv4_os.data(), ipv4_os.size()};

        ipv4_os.reset();
        write_ipv4_libc(ipv4_os, ip);
        std::string libc_str {ipv4_os.data(), ipv4_os.size()};

        // min size = 9
        // 0.0.0.0:0

        // max size = 21
        // 255.255.255.255:65535

        BOOST_CHECK(our_str.size() >= 9);
        BOOST_CHECK(libc_str.size() >= 9);
        BOOST_CHECK(our_str.size() <= 21);
        BOOST_CHECK(libc_str.size() <= 21);
        BOOST_CHECK_EQUAL(our_str, libc_str);
    }
}

BOOST_AUTO_TEST_SUITE_END()
