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

#include "Frame.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(FrameSuite)

BOOST_AUTO_TEST_CASE(GetLengthCase)
{
    constexpr std::uint32_t Length{0x04030201};
    // Little endian encoding.
    BOOST_TEST(get_length("\001\002\003\004") == Length);
}

BOOST_AUTO_TEST_CASE(PutLengthCase)
{
    constexpr std::uint32_t Length{0x04030201};
    char buf[4]{};
    put_length(buf, Length);
    BOOST_TEST(get_length(buf) == Length);
}

BOOST_AUTO_TEST_CASE(ParseFrameCase)
{
    int msg_count{0};
    string msg_data;
    size_t consumed{0};
    auto fn = [&](auto msg) {
        ++msg_count;
        msg_data.append(buffer_cast<const char*>(msg), buffer_size(msg));
    };

    consumed += parse_frame("\003\000\000"sv, fn);
    BOOST_TEST(consumed == 0);
    BOOST_TEST(msg_count == 0);
    BOOST_TEST(msg_data.empty());

    consumed += parse_frame("\003\000\000\000F"sv, fn);
    BOOST_TEST(consumed == 0);
    BOOST_TEST(msg_count == 0);
    BOOST_TEST(msg_data.empty());

    consumed += parse_frame("\003\000\000\000Foo"sv, fn);
    BOOST_TEST(consumed == 7);
    BOOST_TEST(msg_count == 1);
    BOOST_TEST(msg_data == "Foo");

    consumed += parse_frame("\006\000\000\000FooBar\003\000\000\000Baz"sv, fn);
    BOOST_TEST(consumed == 24);
    BOOST_TEST(msg_count == 3);
    BOOST_TEST(msg_data == "FooFooBarBaz");
}

BOOST_AUTO_TEST_SUITE_END()
