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

BOOST_AUTO_TEST_SUITE(SocketSuite)

BOOST_AUTO_TEST_CASE(GetUnixAddrInfoCase)
{
    auto path = "/tmp/foo.sock"s;
    const auto ai = get_unix_addrinfo(path, SOCK_DGRAM);
    BOOST_TEST(ai->ai_flags == 0);
    BOOST_TEST(ai->ai_family == AF_UNIX);
    BOOST_TEST(ai->ai_socktype == SOCK_DGRAM);
    BOOST_TEST(ai->ai_protocol == 0);
    BOOST_TEST(ai->ai_addrlen == offsetof(sockaddr_un, sun_path) + path.size() + 1);
    BOOST_TEST(ai->ai_addr->sa_family == AF_UNIX);
    BOOST_TEST(!ai->ai_canonname);
    BOOST_TEST(!ai->ai_next);
    BOOST_TEST(to_string(*ai->ai_addr) == path);

    path.assign(sizeof(sockaddr_un{}.sun_path), 'x');
    BOOST_CHECK_THROW(get_unix_addrinfo(path, SOCK_DGRAM), invalid_argument);
}

BOOST_AUTO_TEST_SUITE_END()
