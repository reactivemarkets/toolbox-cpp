// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2023 Reactive Markets Limited
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

BOOST_AUTO_TEST_CASE(PutLengthCase)
{
    {
        constexpr uint16_t Length{0x0102};
        char buf[2]{};
        detail::put_length<endian::little>(buf, Length, endian::little);
        BOOST_CHECK_EQUAL(detail::get_length<endian::little>(buf, endian::little), Length);
    }
    {
        constexpr uint16_t Length{0x0102};
        char buf[2]{};
        detail::put_length<endian::little>(buf, Length, endian::big);
        BOOST_CHECK_EQUAL(detail::get_length<endian::little>(buf, endian::big), Length);
    }
    {
        constexpr uint16_t Length{0x0201};
        char buf[2]{};
        detail::put_length<endian::big>(buf, Length, endian::big);
        BOOST_CHECK_EQUAL(detail::get_length<endian::big>(buf, endian::big), Length);
    }
    {
        constexpr uint16_t Length{0x0201};
        char buf[2]{};
        detail::put_length<endian::big>(buf, Length, endian::little);
        BOOST_CHECK_EQUAL(detail::get_length<endian::big>(buf, endian::little), Length);
    }
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

    consumed += parse_frame("\003"sv, fn, endian::little);
    BOOST_CHECK_EQUAL(consumed, 0);
    BOOST_CHECK_EQUAL(msg_count, 0);
    BOOST_CHECK(msg_data.empty());

    consumed += parse_frame("\000\003"
                            "F"sv,
                            fn, endian::big);
    BOOST_CHECK_EQUAL(consumed, 3);
    BOOST_CHECK_EQUAL(msg_count, 1);
    BOOST_CHECK_EQUAL(msg_data, "F");

    consumed += parse_frame("\004\000"
                            "GH"sv,
                            fn, endian::little);
    BOOST_CHECK_EQUAL(consumed, 3 + 4);
    BOOST_CHECK_EQUAL(msg_count, 1 + 1);
    BOOST_CHECK_EQUAL(msg_data,
                      "F"
                      "GH");

    // octal(\010) = decimal(8)
    consumed += parse_frame("\000\010"
                            "FooBar"
                            "\000\005"
                            "Baz"sv,
                            fn, endian::big);
    BOOST_CHECK_EQUAL(consumed, 3 + 4 + 8 + 5);
    BOOST_CHECK_EQUAL(msg_count, 1 + 1 + 2);
    BOOST_CHECK_EQUAL(msg_data,
                      "F"
                      "GH"
                      "FooBar"
                      "Baz");
}

BOOST_AUTO_TEST_SUITE_END()
