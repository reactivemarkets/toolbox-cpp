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

#include <toolbox/net/Endpoint.hpp>
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

BOOST_AUTO_TEST_CASE(AbstractUnixSocketCase)
{
    // From unix(7) man page
    // If a bind(2) call specifies addrlen as sizeof(sa_family_t), or the SO_PASSCRED socket option
    // was specified for a socket that was not explicitly bound to an address, then the socket is
    // auto_bound to an abstract address. The address consists of a null byte followed by 5 bytes in
    // the character set [0-9a-f]. Thus, there is a limit of 2^20 autobind addresses. (From Linux
    // 2.1.15, when the autobind feature was added, 8 bytes were used, and the limit was thus 2^32
    // autobind addresses. The change to 5 bytes came in Linux 2.3.15.)

    FileHandle acc_sock, server_sock, client_sock;
    BOOST_REQUIRE_NO_THROW(acc_sock = os::socket(AF_UNIX, SOCK_STREAM, 0));
    sockaddr_un addr{.sun_family = AF_UNIX, .sun_path = {0}};
    BOOST_REQUIRE_NO_THROW(
        os::bind(acc_sock.get(), *reinterpret_cast<sockaddr*>(&addr), sizeof(addr.sun_family)));
    // get the auto-bound address
    StreamEndpoint auto_bound_endpoint;
    BOOST_REQUIRE_NO_THROW(os::getsockname(acc_sock.get(), auto_bound_endpoint));
    BOOST_CHECK_EQUAL(auto_bound_endpoint.size(), 8);
    BOOST_REQUIRE_NO_THROW(os::listen(acc_sock.get(), 1));
    // connect the client
    BOOST_REQUIRE_NO_THROW(client_sock = os::socket(auto_bound_endpoint.protocol()));
    BOOST_REQUIRE_NO_THROW(os::connect(client_sock.get(), auto_bound_endpoint));
    // accept the connection
    BOOST_REQUIRE_NO_THROW(server_sock = os::accept(acc_sock.get()));

    std::string msg_sent = "Hello";
    BOOST_REQUIRE_EQUAL(
        ssize_t(msg_sent.size()),
        os::send(client_sock.get(), msg_sent.data(), msg_sent.size(), MSG_DONTWAIT));
    std::string msg_recv(msg_sent.size(), '\0');
    BOOST_REQUIRE_NO_THROW(os::recv(server_sock.get(), const_cast<char*>(msg_recv.data()),
                                    msg_sent.size(), MSG_DONTWAIT));
    BOOST_CHECK_EQUAL(msg_sent, msg_recv);
}

BOOST_AUTO_TEST_SUITE_END()
