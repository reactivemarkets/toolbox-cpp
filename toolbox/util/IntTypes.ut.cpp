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

#include "IntTypes.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;
using namespace toolbox;

namespace {

struct TestTag : Int32Policy {
};
using Test = IntWrapper<TestTag>;

constexpr Test operator""_test(unsigned long long val) noexcept
{
    return Test{val};
}

} // namespace

BOOST_AUTO_TEST_SUITE(IntTypesSuite)

BOOST_AUTO_TEST_CASE(IntAddAssignCase)
{
    auto x = 5_test;
    BOOST_TEST((x += 3_test) == 8_test);
    BOOST_TEST(x == 8_test);
}

BOOST_AUTO_TEST_CASE(IntSubAssignCase)
{
    auto x = 5_test;
    BOOST_TEST((x -= 3_test) == 2_test);
    BOOST_TEST(x == 2_test);
}

BOOST_AUTO_TEST_CASE(IntMulAssignCase)
{
    auto x = 5_test;
    BOOST_TEST((x *= 3_test) == 15_test);
    BOOST_TEST(x == 15_test);
}

BOOST_AUTO_TEST_CASE(IntDivAssignCase)
{
    auto x = 15_test;
    BOOST_TEST((x /= 3_test) == 5_test);
    BOOST_TEST(x == 5_test);
}

BOOST_AUTO_TEST_CASE(IntModAssignCase)
{
    auto x = 5_test;
    BOOST_TEST((x %= 3_test) == 2_test);
    BOOST_TEST(x == 2_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseAndAssignCase)
{
    // 11100
    // 00111
    // 00100
    auto x = 28_test;
    BOOST_TEST((x &= 7_test) == 4_test);
    BOOST_TEST(x == 4_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseOrAssignCase)
{
    // 11100
    // 00111
    // 11111
    auto x = 28_test;
    BOOST_TEST((x |= 7_test) == 31_test);
    BOOST_TEST(x == 31_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseXorAssignCase)
{
    // 11100
    // 00111
    // 11011
    auto x = 28_test;
    BOOST_TEST((x ^= 7_test) == 27_test);
    BOOST_TEST(x == 27_test);
}

BOOST_AUTO_TEST_CASE(IntLeftShiftAssignCase)
{
    // 00111
    // 11100
    auto x = 7_test;
    BOOST_TEST((x <<= 2_test) == 28_test);
    BOOST_TEST(x == 28_test);
}

BOOST_AUTO_TEST_CASE(IntRightShiftAssignCase)
{
    // 11100
    // 00111
    auto x = 28_test;
    BOOST_TEST((x >>= 2_test) == 7_test);
    BOOST_TEST(x == 7_test);
}

BOOST_AUTO_TEST_CASE(IntPreIncCase)
{
    auto x = 5_test;
    BOOST_TEST(++x == 6_test);
    BOOST_TEST(x == 6_test);
}

BOOST_AUTO_TEST_CASE(IntPreDecCase)
{
    auto x = 5_test;
    BOOST_TEST(--x == 4_test);
    BOOST_TEST(x == 4_test);
}

BOOST_AUTO_TEST_CASE(IntPostIncCase)
{
    auto x = 5_test;
    BOOST_TEST(x++ == 5_test);
    BOOST_TEST(x == 6_test);
}

BOOST_AUTO_TEST_CASE(IntPostDecCase)
{
    auto x = 5_test;
    BOOST_TEST(x-- == 5_test);
    BOOST_TEST(x == 4_test);
}

BOOST_AUTO_TEST_CASE(IntUnaryPlusCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST(+x == 5_test);
}

BOOST_AUTO_TEST_CASE(IntUnaryMinusCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST(-x == -5_test);
}

BOOST_AUTO_TEST_CASE(IntAddCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST((x + 3_test) == 8_test);
}

BOOST_AUTO_TEST_CASE(IntSubCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST((x - 3_test) == 2_test);
}

BOOST_AUTO_TEST_CASE(IntMulCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST((x * 3_test) == 15_test);
}

BOOST_AUTO_TEST_CASE(IntDivCase)
{
    constexpr auto x = 15_test;
    BOOST_TEST(x / 3_test == 5_test);
}

BOOST_AUTO_TEST_CASE(IntModCase)
{
    constexpr auto x = 5_test;
    BOOST_TEST((x % 3_test) == 2_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseNotCase)
{
    constexpr auto x = Test{~1};
    BOOST_TEST(~x == 1_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseAndCase)
{
    // 11100
    // 00111
    // 00100
    constexpr auto x = 28_test;
    BOOST_TEST((x & 7_test) == 4_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseOrCase)
{
    // 11100
    // 00111
    // 11111
    constexpr auto x = 28_test;
    BOOST_TEST((x | 7_test) == 31_test);
}

BOOST_AUTO_TEST_CASE(IntBitwiseXorCase)
{
    // 11100
    // 00111
    // 11011
    constexpr auto x = 28_test;
    BOOST_TEST((x ^ 7_test) == 27_test);
}

BOOST_AUTO_TEST_CASE(IntLeftShiftCase)
{
    // 00111
    // 11100
    constexpr auto x = 7_test;
    BOOST_TEST((x << 2_test) == 28_test);
}

BOOST_AUTO_TEST_CASE(IntRightShiftCase)
{
    // 11100
    // 00111
    constexpr auto x = 28_test;
    BOOST_TEST((x >> 2_test) == 7_test);
}

BOOST_AUTO_TEST_CASE(IntEqualToCase)
{
    BOOST_TEST(5_test == 5_test);
}

BOOST_AUTO_TEST_CASE(IntNotEqualToCase)
{
    BOOST_TEST(5_test != 7_test);
}

BOOST_AUTO_TEST_CASE(IntLessThanCase)
{
    BOOST_TEST(5_test < 7_test);
}

BOOST_AUTO_TEST_CASE(IntGreaterThanCase)
{
    BOOST_TEST(5_test > 3_test);
}

BOOST_AUTO_TEST_CASE(IntLessThanOrEqualToCase)
{
    BOOST_TEST(5_test <= 5_test);
    BOOST_TEST(5_test <= 7_test);
}

BOOST_AUTO_TEST_CASE(IntGreaterThanOrEqualToCase)
{
    BOOST_TEST(5_test >= 5_test);
    BOOST_TEST(5_test >= 3_test);
}

BOOST_AUTO_TEST_CASE(IntInsertionCase)
{
    BOOST_TEST(lexical_cast<string>(5_test) == "5");
}

BOOST_AUTO_TEST_SUITE_END()
