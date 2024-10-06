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

#include "Utility.hpp"
#include <cmath>
#include <limits>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(UtilitySuite)

BOOST_AUTO_TEST_CASE(StobCase)
{
    BOOST_CHECK_EQUAL(stob(""sv, false), false);
    BOOST_CHECK_EQUAL(stob(""sv, true), true);

    BOOST_CHECK_EQUAL(stob("0"sv, true), false);
    BOOST_CHECK_EQUAL(stob("F"sv, true), false);
    BOOST_CHECK_EQUAL(stob("N"sv, true), false);
    BOOST_CHECK_EQUAL(stob("f"sv, true), false);
    BOOST_CHECK_EQUAL(stob("n"sv, true), false);

    BOOST_CHECK_EQUAL(stob("1"sv, false), true);
    BOOST_CHECK_EQUAL(stob("T"sv, false), true);
    BOOST_CHECK_EQUAL(stob("Y"sv, false), true);
    BOOST_CHECK_EQUAL(stob("t"sv, false), true);
    BOOST_CHECK_EQUAL(stob("y"sv, false), true);

    BOOST_CHECK_EQUAL(stob("NO"sv, true), false);
    BOOST_CHECK_EQUAL(stob("No"sv, true), false);
    BOOST_CHECK_EQUAL(stob("no"sv, true), false);

    BOOST_CHECK_EQUAL(stob("ON"sv, false), true);
    BOOST_CHECK_EQUAL(stob("On"sv, false), true);
    BOOST_CHECK_EQUAL(stob("on"sv, false), true);

    BOOST_CHECK_EQUAL(stob("OFF"sv, true), false);
    BOOST_CHECK_EQUAL(stob("Off"sv, true), false);
    BOOST_CHECK_EQUAL(stob("off"sv, true), false);

    BOOST_CHECK_EQUAL(stob("YES"sv, false), true);
    BOOST_CHECK_EQUAL(stob("Yes"sv, false), true);
    BOOST_CHECK_EQUAL(stob("yes"sv, false), true);

    BOOST_CHECK_EQUAL(stob("TRUE"sv, false), true);
    BOOST_CHECK_EQUAL(stob("True"sv, false), true);
    BOOST_CHECK_EQUAL(stob("true"sv, false), true);

    BOOST_CHECK_EQUAL(stob("FALSE"sv, true), false);
    BOOST_CHECK_EQUAL(stob("False"sv, true), false);
    BOOST_CHECK_EQUAL(stob("false"sv, true), false);
}

BOOST_AUTO_TEST_CASE(StodCase, *utf::tolerance(0.0000001))
{
    BOOST_CHECK_EQUAL(stod(""sv), 0);
    BOOST_CHECK_EQUAL(stod("1"sv), 1);
    BOOST_CHECK_EQUAL(stod("-1"sv), -1);
    BOOST_CHECK_EQUAL(stod("1.23E2"sv), 123);
    BOOST_CHECK_EQUAL(stod("-1.23E2"sv), -123);
    BOOST_CHECK_EQUAL(stod("1.23E1"sv), 12.3);
    BOOST_CHECK_EQUAL(stod("-1.23E1"sv), -12.3);
    BOOST_CHECK_EQUAL(stod("1.23"sv), 1.23);
    BOOST_CHECK_EQUAL(stod("-1.23"sv), -1.23);

    // with default value.
    BOOST_CHECK(isnan(stod(""sv, numeric_limits<double>::quiet_NaN())));
    BOOST_CHECK(isnan(stod("abc"sv, numeric_limits<double>::quiet_NaN())));
    BOOST_CHECK_EQUAL(stod("1"sv, numeric_limits<double>::quiet_NaN()), 1);
}

BOOST_AUTO_TEST_CASE(StonDoubleCase, *utf::tolerance(0.0000001))
{
    BOOST_CHECK_EQUAL(ston<double>(""sv), 0);
    BOOST_CHECK_EQUAL(ston<double>("1"sv), 1);
    BOOST_CHECK_EQUAL(ston<double>("-1"sv), -1);
    BOOST_CHECK_EQUAL(ston<double>("1.23E2"sv), 123);
    BOOST_CHECK_EQUAL(ston<double>("-1.23E2"sv), -123);
    BOOST_CHECK_EQUAL(ston<double>("1.23E1"sv), 12.3);
    BOOST_CHECK_EQUAL(ston<double>("-1.23E1"sv), -12.3);
    BOOST_CHECK_EQUAL(ston<double>("1.23"sv), 1.23);
    BOOST_CHECK_EQUAL(ston<double>("-1.23"sv), -1.23);
}

BOOST_AUTO_TEST_CASE(StonSubstrCase)
{
    constexpr auto s = "12345"sv;
    BOOST_CHECK_EQUAL(ston<double>(s.substr(0, 3)), 123);
    BOOST_CHECK_EQUAL(ston<int>(s.substr(0, 3)), 123);
}

BOOST_AUTO_TEST_CASE(Stoi16Case)
{
    BOOST_CHECK_EQUAL(ston<int16_t>(""sv), 0);
    BOOST_CHECK_EQUAL(ston<int16_t>("1"sv), 1);
    BOOST_CHECK_EQUAL(ston<int16_t>("123"sv), 123);
    BOOST_CHECK_EQUAL(ston<int16_t>(" "sv), 0);
    BOOST_CHECK_EQUAL(ston<int16_t>("x"sv), 0);
    BOOST_CHECK_EQUAL(ston<int16_t>("x1 "sv), 0);
    BOOST_CHECK_EQUAL(ston<int16_t>("x123 "sv), 0);
    BOOST_CHECK_EQUAL(ston<int16_t>("32767"sv), 0x7fff);
    BOOST_CHECK_EQUAL(ston<int16_t>("-32768"sv), -0x8000);
}

BOOST_AUTO_TEST_CASE(Stoi32Case)
{
    BOOST_CHECK_EQUAL(ston<int32_t>(""sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("1"sv), 1);
    BOOST_CHECK_EQUAL(ston<int32_t>("123"sv), 123);
    BOOST_CHECK_EQUAL(ston<int32_t>(" "sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("x"sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("x1 "sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("x123 "sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("2147483647"sv), 0x7fffffff);
    BOOST_CHECK_EQUAL(ston<int32_t>("-2147483648"sv), -0x80000000);
}

BOOST_AUTO_TEST_CASE(Stoi32NegSignCase)
{
    BOOST_CHECK_EQUAL(ston<int32_t>("-"sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("-1"sv), -1);
    BOOST_CHECK_EQUAL(ston<int32_t>("-123"sv), -123);
    BOOST_CHECK_EQUAL(ston<int32_t>("-x"sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("x-1 "sv), 0);
    BOOST_CHECK_EQUAL(ston<int32_t>("x-123 "sv), 0);
}

BOOST_AUTO_TEST_CASE(Stoi64Case)
{
    BOOST_CHECK_EQUAL(ston<int64_t>(""sv), 0L);
    BOOST_CHECK_EQUAL(ston<int64_t>("1"sv), 1L);
    BOOST_CHECK_EQUAL(ston<int64_t>("123"sv), 123L);
    BOOST_CHECK_EQUAL(ston<int64_t>(" "sv), 0L);
    BOOST_CHECK_EQUAL(ston<int64_t>("x"sv), 0L);
    BOOST_CHECK_EQUAL(ston<int64_t>("x1 "sv), 0L);
    BOOST_CHECK_EQUAL(ston<int64_t>("x123 "sv), 0L);
    BOOST_CHECK_EQUAL(ston<int64_t>("9223372036854775807"sv), 0x7fffffffffffffffLL);
    BOOST_CHECK_EQUAL(ston<int64_t>("-9223372036854775808"sv), -0x8000000000000000LL);
}

BOOST_AUTO_TEST_CASE(Stou16Case)
{
    BOOST_CHECK_EQUAL(ston<uint16_t>(""sv), 0);
    BOOST_CHECK_EQUAL(ston<uint16_t>("1"sv), 1);
    BOOST_CHECK_EQUAL(ston<uint16_t>("123"sv), 123);
    BOOST_CHECK_EQUAL(ston<uint16_t>(" "sv), 0);
    BOOST_CHECK_EQUAL(ston<uint16_t>("x"sv), 0);
    BOOST_CHECK_EQUAL(ston<uint16_t>("x1 "sv), 0);
    BOOST_CHECK_EQUAL(ston<uint16_t>("x123 "sv), 0);
    BOOST_CHECK_EQUAL(ston<uint16_t>("65535"sv), 0xffff);
}

BOOST_AUTO_TEST_CASE(Stou32Case)
{
    BOOST_CHECK_EQUAL(ston<uint32_t>(""sv), 0U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("1"sv), 1U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("123"sv), 123U);
    BOOST_CHECK_EQUAL(ston<uint32_t>(" "sv), 0U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("x"sv), 0U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("x1 "sv), 0U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("x123 "sv), 0U);
    BOOST_CHECK_EQUAL(ston<uint32_t>("4294967295"sv), 0xffffffffU);
}

BOOST_AUTO_TEST_CASE(Stou64Case)
{
    BOOST_CHECK_EQUAL(ston<uint64_t>(""sv), 0UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("1"sv), 1UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("123"sv), 123UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>(" "sv), 0UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("x"sv), 0UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("x1 "sv), 0UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("x123 "sv), 0UL);
    BOOST_CHECK_EQUAL(ston<uint64_t>("18446744073709551615"sv), 0xffffffffffffffffULL);
}

BOOST_AUTO_TEST_CASE(HexDigitsCase)
{
    BOOST_CHECK_EQUAL(hex_digits(0x0U), 1);
    BOOST_CHECK_EQUAL(hex_digits(0x1U), 1);
    BOOST_CHECK_EQUAL(hex_digits(0xfU), 1);
    BOOST_CHECK_EQUAL(hex_digits(0x10U), 2);
    BOOST_CHECK_EQUAL(hex_digits(0xffU), 2);

    BOOST_CHECK_EQUAL(hex_digits(0xcdefU), 4);
    BOOST_CHECK_EQUAL(hex_digits(0x10000U), 5);
    BOOST_CHECK_EQUAL(hex_digits(0x89abcdefU), 8);
    BOOST_CHECK_EQUAL(hex_digits(0x100000000U), 9);
    BOOST_CHECK_EQUAL(hex_digits(0x567890abcdefU), 12);
    BOOST_CHECK_EQUAL(hex_digits(0x1000000000000U), 13);
    BOOST_CHECK_EQUAL(hex_digits(0x1234567890abcdefU), 16);
}

BOOST_AUTO_TEST_CASE(DecDigitsUnsignedCase)
{
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{0}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{11}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{101}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1001}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10001}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99999}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100001}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999999}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000001}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999999}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000001}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99999999}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000001}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999999999}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000001}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999999999}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000001}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99999999999}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000000}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000001}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999999999999}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000000}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000001}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999999999999}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000000}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000001}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99999999999999}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000000000}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000000001}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999999999999999}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000000000}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000000001}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999999999999999}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000000000}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000000001}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{99999999999999999}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000000000000}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{100000000000000001}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{999999999999999999}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000000000000}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{1000000000000000001}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{9999999999999999999ULL}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000000000000ULL}), 20);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{10000000000000000001ULL}), 20);
    BOOST_CHECK_EQUAL(dec_digits(std::uint64_t{18446744073709551615ULL}), 20);
}

BOOST_AUTO_TEST_CASE(DecDigitSignedCase)
{
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-11}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-101}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1001}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9999}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10001}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99999}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100001}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999999}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000001}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9999999}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000001}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99999999}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000001}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999999999}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000001}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9999999999}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000000}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000001}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99999999999}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000000}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000001}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999999999999}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000000}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000001}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9999999999999}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000000000}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000000001}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99999999999999}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000000000}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000000001}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999999999999999}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000000000}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000000001}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9999999999999999}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000000000000}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-10000000000000001}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-99999999999999999}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000000000000}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-100000000000000001}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-999999999999999999}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000000000000}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-1000000000000000001}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{-9223372036854775807}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::numeric_limits<std::int64_t>::min()), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{0}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9}), 1);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{11}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99}), 2);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{101}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999}), 3);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1001}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9999}), 4);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10001}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99999}), 5);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100001}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999999}), 6);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000001}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9999999}), 7);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000001}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99999999}), 8);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000001}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999999999}), 9);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000001}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9999999999}), 10);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000000}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000001}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99999999999}), 11);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000000}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000001}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999999999999}), 12);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000000}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000001}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9999999999999}), 13);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000000000}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000000001}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99999999999999}), 14);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000000000}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000000001}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999999999999999}), 15);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000000000}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000000001}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9999999999999999}), 16);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000000000000}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{10000000000000001}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{99999999999999999}), 17);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000000000000}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{100000000000000001}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{999999999999999999}), 18);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000000000000}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{1000000000000000001}), 19);
    BOOST_CHECK_EQUAL(dec_digits(std::int64_t{9223372036854775807}), 19);
}

BOOST_AUTO_TEST_SUITE_END()
