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

#include "Math.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(MathSuite)

BOOST_AUTO_TEST_CASE(NextPow2Case)
{
    BOOST_TEST(next_pow2(1U) == 1U);
    BOOST_TEST(next_pow2(2U) == 2U);
    BOOST_TEST(next_pow2(3U) == 4U);
    BOOST_TEST(next_pow2(4U) == 4U);
    BOOST_TEST(next_pow2(5U) == 8U);
    BOOST_TEST(next_pow2(6U) == 8U);
    BOOST_TEST(next_pow2(7U) == 8U);
    BOOST_TEST(next_pow2(8U) == 8U);

    BOOST_TEST(next_pow2(1UL) == 1UL);
    BOOST_TEST(next_pow2(2UL) == 2UL);
    BOOST_TEST(next_pow2(3UL) == 4UL);
    BOOST_TEST(next_pow2(4UL) == 4UL);
    BOOST_TEST(next_pow2(5UL) == 8UL);
    BOOST_TEST(next_pow2(6UL) == 8UL);
    BOOST_TEST(next_pow2(7UL) == 8UL);
    BOOST_TEST(next_pow2(8UL) == 8UL);
}

BOOST_AUTO_TEST_CASE(VarAccumCase, *utf::tolerance(0.0000001))
{
    VarAccum v;
    v.append(1345, 1301, 1368, 1322, 1310, 1370, 1318, 1350, 1303, 1299);
    BOOST_TEST(v.size() == 10U);
    BOOST_TEST(v.mean() == 1328.6);
    BOOST_TEST(var(v) == 754.2666667);
    BOOST_TEST(varp(v) == 678.84);
    BOOST_TEST(stdev(v) == 27.4639157);
    BOOST_TEST(stdevp(v) == 26.0545581);
    BOOST_TEST(v.min() == 1299);
    BOOST_TEST(v.max() == 1370);
}

BOOST_AUTO_TEST_CASE(CeilCase)
{
    BOOST_TEST(ceil(1U, 3U) == 1U);
    BOOST_TEST(ceil(2U, 3U) == 1U);
    BOOST_TEST(ceil(3U, 3U) == 1U);
    BOOST_TEST(ceil(4U, 3U) == 2U);
    BOOST_TEST(ceil(5U, 3U) == 2U);
    BOOST_TEST(ceil(6U, 3U) == 2U);
}

BOOST_AUTO_TEST_SUITE_END()
