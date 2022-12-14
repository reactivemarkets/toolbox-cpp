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

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(UtilitySuite)

BOOST_AUTO_TEST_CASE(StobCase)
{
    BOOST_TEST(stob(""sv, false) == false);
    BOOST_TEST(stob(""sv, true) == true);

    BOOST_TEST(stob("0"sv, true) == false);
    BOOST_TEST(stob("F"sv, true) == false);
    BOOST_TEST(stob("N"sv, true) == false);
    BOOST_TEST(stob("f"sv, true) == false);
    BOOST_TEST(stob("n"sv, true) == false);

    BOOST_TEST(stob("1"sv, false) == true);
    BOOST_TEST(stob("T"sv, false) == true);
    BOOST_TEST(stob("Y"sv, false) == true);
    BOOST_TEST(stob("t"sv, false) == true);
    BOOST_TEST(stob("y"sv, false) == true);

    BOOST_TEST(stob("NO"sv, true) == false);
    BOOST_TEST(stob("No"sv, true) == false);
    BOOST_TEST(stob("no"sv, true) == false);

    BOOST_TEST(stob("ON"sv, false) == true);
    BOOST_TEST(stob("On"sv, false) == true);
    BOOST_TEST(stob("on"sv, false) == true);

    BOOST_TEST(stob("OFF"sv, true) == false);
    BOOST_TEST(stob("Off"sv, true) == false);
    BOOST_TEST(stob("off"sv, true) == false);

    BOOST_TEST(stob("YES"sv, false) == true);
    BOOST_TEST(stob("Yes"sv, false) == true);
    BOOST_TEST(stob("yes"sv, false) == true);

    BOOST_TEST(stob("TRUE"sv, false) == true);
    BOOST_TEST(stob("True"sv, false) == true);
    BOOST_TEST(stob("true"sv, false) == true);

    BOOST_TEST(stob("FALSE"sv, true) == false);
    BOOST_TEST(stob("False"sv, true) == false);
    BOOST_TEST(stob("false"sv, true) == false);
}

BOOST_AUTO_TEST_CASE(StodCase, *utf::tolerance(0.0000001))
{
    BOOST_TEST(stod(""sv) == 0);
    BOOST_TEST(stod("1"sv) == 1);
    BOOST_TEST(stod("-1"sv) == -1);
    BOOST_TEST(stod("1.23E2"sv) == 123);
    BOOST_TEST(stod("-1.23E2"sv) == -123);
    BOOST_TEST(stod("1.23E1"sv) == 12.3);
    BOOST_TEST(stod("-1.23E1"sv) == -12.3);
    BOOST_TEST(stod("1.23"sv) == 1.23);
    BOOST_TEST(stod("-1.23"sv) == -1.23);

    // with default value.
    BOOST_TEST(isnan(stod(""sv, numeric_limits<double>::quiet_NaN())));
    BOOST_TEST(isnan(stod("abc"sv, numeric_limits<double>::quiet_NaN())));
    BOOST_TEST(stod("1"sv, numeric_limits<double>::quiet_NaN()) == 1);
}

BOOST_AUTO_TEST_CASE(StonDoubleCase, *utf::tolerance(0.0000001))
{
    BOOST_TEST(ston<double>(""sv) == 0);
    BOOST_TEST(ston<double>("1"sv) == 1);
    BOOST_TEST(ston<double>("-1"sv) == -1);
    BOOST_TEST(ston<double>("1.23E2"sv) == 123);
    BOOST_TEST(ston<double>("-1.23E2"sv) == -123);
    BOOST_TEST(ston<double>("1.23E1"sv) == 12.3);
    BOOST_TEST(ston<double>("-1.23E1"sv) == -12.3);
    BOOST_TEST(ston<double>("1.23"sv) == 1.23);
    BOOST_TEST(ston<double>("-1.23"sv) == -1.23);
}

BOOST_AUTO_TEST_CASE(StonSubstrCase)
{
    constexpr auto s = "12345"sv;
    BOOST_TEST(ston<double>(s.substr(0, 3)) == 123);
    BOOST_TEST(ston<int>(s.substr(0, 3)) == 123);
}

BOOST_AUTO_TEST_CASE(Stoi16Case)
{
    BOOST_TEST(ston<int16_t>(""sv) == 0);
    BOOST_TEST(ston<int16_t>("1"sv) == 1);
    BOOST_TEST(ston<int16_t>("123"sv) == 123);
    BOOST_TEST(ston<int16_t>(" "sv) == 0);
    BOOST_TEST(ston<int16_t>("x"sv) == 0);
    BOOST_TEST(ston<int16_t>("x1 "sv) == 0);
    BOOST_TEST(ston<int16_t>("x123 "sv) == 0);
    BOOST_TEST(ston<int16_t>("32767"sv) == 0x7fff);
    BOOST_TEST(ston<int16_t>("-32768"sv) == -0x8000);
}

BOOST_AUTO_TEST_CASE(Stoi32Case)
{
    BOOST_TEST(ston<int32_t>(""sv) == 0);
    BOOST_TEST(ston<int32_t>("1"sv) == 1);
    BOOST_TEST(ston<int32_t>("123"sv) == 123);
    BOOST_TEST(ston<int32_t>(" "sv) == 0);
    BOOST_TEST(ston<int32_t>("x"sv) == 0);
    BOOST_TEST(ston<int32_t>("x1 "sv) == 0);
    BOOST_TEST(ston<int32_t>("x123 "sv) == 0);
    BOOST_TEST(ston<int32_t>("2147483647"sv) == 0x7fffffff);
    BOOST_TEST(ston<int32_t>("-2147483648"sv) == -0x80000000);
}

BOOST_AUTO_TEST_CASE(Stoi32NegSignCase)
{
    BOOST_TEST(ston<int32_t>("-"sv) == 0);
    BOOST_TEST(ston<int32_t>("-1"sv) == -1);
    BOOST_TEST(ston<int32_t>("-123"sv) == -123);
    BOOST_TEST(ston<int32_t>("-x"sv) == 0);
    BOOST_TEST(ston<int32_t>("x-1 "sv) == 0);
    BOOST_TEST(ston<int32_t>("x-123 "sv) == 0);
}

BOOST_AUTO_TEST_CASE(Stoi64Case)
{
    BOOST_TEST(ston<int64_t>(""sv) == 0L);
    BOOST_TEST(ston<int64_t>("1"sv) == 1L);
    BOOST_TEST(ston<int64_t>("123"sv) == 123L);
    BOOST_TEST(ston<int64_t>(" "sv) == 0L);
    BOOST_TEST(ston<int64_t>("x"sv) == 0L);
    BOOST_TEST(ston<int64_t>("x1 "sv) == 0L);
    BOOST_TEST(ston<int64_t>("x123 "sv) == 0L);
    BOOST_TEST(ston<int64_t>("9223372036854775807"sv) == 0x7fffffffffffffffLL);
    BOOST_TEST(ston<int64_t>("-9223372036854775808"sv) == -0x8000000000000000LL);
}

BOOST_AUTO_TEST_CASE(Stou16Case)
{
    BOOST_TEST(ston<uint16_t>(""sv) == 0);
    BOOST_TEST(ston<uint16_t>("1"sv) == 1);
    BOOST_TEST(ston<uint16_t>("123"sv) == 123);
    BOOST_TEST(ston<uint16_t>(" "sv) == 0);
    BOOST_TEST(ston<uint16_t>("x"sv) == 0);
    BOOST_TEST(ston<uint16_t>("x1 "sv) == 0);
    BOOST_TEST(ston<uint16_t>("x123 "sv) == 0);
    BOOST_TEST(ston<uint16_t>("65535"sv) == 0xffff);
}

BOOST_AUTO_TEST_CASE(Stou32Case)
{
    BOOST_TEST(ston<uint32_t>(""sv) == 0U);
    BOOST_TEST(ston<uint32_t>("1"sv) == 1U);
    BOOST_TEST(ston<uint32_t>("123"sv) == 123U);
    BOOST_TEST(ston<uint32_t>(" "sv) == 0U);
    BOOST_TEST(ston<uint32_t>("x"sv) == 0U);
    BOOST_TEST(ston<uint32_t>("x1 "sv) == 0U);
    BOOST_TEST(ston<uint32_t>("x123 "sv) == 0U);
    BOOST_TEST(ston<uint32_t>("4294967295"sv) == 0xffffffffU);
}

BOOST_AUTO_TEST_CASE(Stou64Case)
{
    BOOST_TEST(ston<uint64_t>(""sv) == 0UL);
    BOOST_TEST(ston<uint64_t>("1"sv) == 1UL);
    BOOST_TEST(ston<uint64_t>("123"sv) == 123UL);
    BOOST_TEST(ston<uint64_t>(" "sv) == 0UL);
    BOOST_TEST(ston<uint64_t>("x"sv) == 0UL);
    BOOST_TEST(ston<uint64_t>("x1 "sv) == 0UL);
    BOOST_TEST(ston<uint64_t>("x123 "sv) == 0UL);
    BOOST_TEST(ston<uint64_t>("18446744073709551615"sv) == 0xffffffffffffffffULL);
}

BOOST_AUTO_TEST_CASE(HexDigitsCase)
{
    BOOST_TEST(hex_digits(0x0U) == 1);
    BOOST_TEST(hex_digits(0x1U) == 1);
    BOOST_TEST(hex_digits(0xfU) == 1);
    BOOST_TEST(hex_digits(0x10U) == 2);
    BOOST_TEST(hex_digits(0xffU) == 2);

    BOOST_TEST(hex_digits(0xcdefU) == 4);
    BOOST_TEST(hex_digits(0x10000U) == 5);
    BOOST_TEST(hex_digits(0x89abcdefU) == 8);
    BOOST_TEST(hex_digits(0x100000000U) == 9);
    BOOST_TEST(hex_digits(0x567890abcdefU) == 12);
    BOOST_TEST(hex_digits(0x1000000000000U) == 13);
    BOOST_TEST(hex_digits(0x1234567890abcdefU) == 16);
}

BOOST_AUTO_TEST_CASE(DecDigitsCase)
{
    BOOST_TEST(dec_digits(0) == 1);
    BOOST_TEST(dec_digits(1) == 1);
    BOOST_TEST(dec_digits(10) == 2);
    BOOST_TEST(dec_digits(100) == 3);
    BOOST_TEST(dec_digits(1000) == 4);
    BOOST_TEST(dec_digits(10000) == 5);
    BOOST_TEST(dec_digits(100000) == 6);
    BOOST_TEST(dec_digits(1000000) == 7);
    BOOST_TEST(dec_digits(10000000) == 8);
    BOOST_TEST(dec_digits(100000000) == 9);
    BOOST_TEST(dec_digits(1000000000) == 10);
    BOOST_TEST(dec_digits(10000000000) == 11);
    BOOST_TEST(dec_digits(100000000000) == 12);
    BOOST_TEST(dec_digits(1000000000000) == 13);
    BOOST_TEST(dec_digits(10000000000000) == 14);
    BOOST_TEST(dec_digits(100000000000000) == 15);
    BOOST_TEST(dec_digits(1000000000000000) == 16);
    BOOST_TEST(dec_digits(10000000000000000) == 17);
    BOOST_TEST(dec_digits(100000000000000000) == 18);
    BOOST_TEST(dec_digits(1000000000000000000) == 19);
}

BOOST_AUTO_TEST_SUITE_END()
