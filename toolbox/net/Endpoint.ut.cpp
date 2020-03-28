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

#include <toolbox/util/String.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(EndpointSuite)

BOOST_AUTO_TEST_CASE(ParseDgramUnspec4Case)
{
    const auto uri = "192.168.1.3:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET);
    BOOST_TEST(ep.protocol().type() == SOCK_DGRAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_UDP);
    BOOST_TEST(to_string(ep) == "udp4://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUnspec6Case)
{
    const auto uri = "[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET6);
    BOOST_TEST(ep.protocol().type() == SOCK_DGRAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_UDP);
    BOOST_TEST(to_string(ep) == "udp6://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUdp4Case)
{
    const auto uri = "udp4://192.168.1.3:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET);
    BOOST_TEST(ep.protocol().type() == SOCK_DGRAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_UDP);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUdp6Case)
{
    const auto uri = "udp6://[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET6);
    BOOST_TEST(ep.protocol().type() == SOCK_DGRAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_UDP);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseDgramUnixCase)
{
    const auto uri = "unix:///tmp/foo.sock"s;
    const auto ep = parse_dgram_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_UNIX);
    BOOST_TEST(ep.protocol().type() == SOCK_DGRAM);
    BOOST_TEST(ep.protocol().protocol() == 0);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnspec4Case)
{
    const auto uri = "192.168.1.3:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_TCP);
    BOOST_TEST(to_string(ep) == "tcp4://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnspec6Case)
{
    const auto uri = "[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET6);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_TCP);
    BOOST_TEST(to_string(ep) == "tcp6://"s + uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamTcp4Case)
{
    const auto uri = "tcp4://192.168.1.3:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_TCP);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamTcp6Case)
{
    const auto uri = "tcp6://[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_INET6);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_TCP);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamUnixCase)
{
    const auto uri = "unix:///tmp/foo.sock"s;
    const auto ep = parse_stream_endpoint(uri);
    BOOST_TEST(ep.protocol().family() == AF_UNIX);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == 0);
    BOOST_TEST(to_string(ep) == uri);
}

BOOST_AUTO_TEST_CASE(ParseStreamBindCase)
{
    const auto ep = parse_stream_endpoint("tcp4://:80");
    BOOST_TEST(ep.protocol().family() == AF_INET);
    BOOST_TEST(ep.protocol().type() == SOCK_STREAM);
    BOOST_TEST(ep.protocol().protocol() == IPPROTO_TCP);
    BOOST_TEST(to_string(ep) == "tcp4://0.0.0.0:80");
}

BOOST_AUTO_TEST_SUITE_END()
