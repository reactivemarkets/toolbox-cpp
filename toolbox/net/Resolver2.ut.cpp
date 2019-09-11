// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#include "Resolver2.hpp"

#include <toolbox/util/Debug.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(Resolver2Suite)

BOOST_AUTO_TEST_CASE(DnsServersConf)
{

    stringstream conf{"# Comment\n"
                      "nameserver 1.1.1.1\n"};

    DnsServers dns_servers{conf};

    BOOST_TEST(dns_servers.server() == "1.1.1.1");
}

BOOST_AUTO_TEST_CASE(DnsNameEnconding)
{
    std::array<char, 30> enc;
    enc.fill(0xcc);

    const std::string host{"www.reactivemarkets.com"};
    to_dns(host, enc.data());

    stringstream ss;
    ss << hex_dump(enc.data(), host.size() + 3,
                   hex_dump::Mode::NON_PRINTABLE); // +3 (0x03 + terminator + first 0xcc)

    BOOST_TEST(ss.str() == " 0x03 w w w 0x0f r e a c t i v e m a r k e t s 0x03 c o m 0x00 0xcc");
}

BOOST_AUTO_TEST_SUITE_END()
