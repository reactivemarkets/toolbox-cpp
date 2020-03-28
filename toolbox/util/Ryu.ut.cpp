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

#include "Ryu.hpp"

#include <boost/test/unit_test.hpp>

using namespace std::literals::string_view_literals;

BOOST_AUTO_TEST_SUITE(RyuSuite)

BOOST_AUTO_TEST_CASE(RyuDtosCase)
{
    using toolbox::dtos;
    BOOST_TEST(dtos(0) == "0"sv);
    BOOST_TEST(dtos(1) == "1"sv);
    BOOST_TEST(dtos(-1) == "-1"sv);
    BOOST_TEST(dtos(123) == "1.23E2"sv);
    BOOST_TEST(dtos(-123) == "-1.23E2"sv);
    BOOST_TEST(dtos(12.3) == "1.23E1"sv);
    BOOST_TEST(dtos(-12.3) == "-1.23E1"sv);
    BOOST_TEST(dtos(1.23) == "1.23"sv);
    BOOST_TEST(dtos(-1.23) == "-1.23"sv);
    BOOST_TEST(dtos(1e6) == "1E6"sv);
    BOOST_TEST(dtos(-1e6) == "-1E6"sv);
    BOOST_TEST(dtos(1.0 / 0.0) == "Infinity"sv);
}

BOOST_AUTO_TEST_CASE(RyuFixedCase)
{
    using toolbox::dtofixed;
    BOOST_TEST(dtofixed(0) == "0"sv);
    BOOST_TEST(dtofixed(1) == "1"sv);
    BOOST_TEST(dtofixed(-1) == "-1"sv);
    BOOST_TEST(dtofixed(123) == "123"sv);
    BOOST_TEST(dtofixed(-123) == "-123"sv);
    BOOST_TEST(dtofixed(12.3) == "12.3"sv);
    BOOST_TEST(dtofixed(-12.3) == "-12.3"sv);
    BOOST_TEST(dtofixed(1.23) == "1.23"sv);
    BOOST_TEST(dtofixed(-1.23) == "-1.23"sv);
    BOOST_TEST(dtofixed(1e6) == "1000000"sv);
    BOOST_TEST(dtofixed(-1e6) == "-1000000"sv);
    BOOST_TEST(dtofixed(1.0 / 0.0) == "Infinity"sv);
    BOOST_TEST(dtofixed(std::numeric_limits<std::int64_t>::max() * 10.0)
               == "92233720368547758080"sv);
    BOOST_TEST(dtofixed(std::numeric_limits<std::int64_t>::min() * 10.0)
               == "-92233720368547758080"sv);
    BOOST_TEST(dtofixed(std::numeric_limits<double>::max()).size() == 309);
    BOOST_TEST(dtofixed(std::numeric_limits<double>::min(), toolbox::MaxRyuPrec).size() == 310);
}

BOOST_AUTO_TEST_SUITE_END()
