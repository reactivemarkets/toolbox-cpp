// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#include "Ryu.hpp"

#include <boost/test/unit_test.hpp>

namespace {
std::string to_string(double f)
{
    char buf[toolbox::MaxRyuBuf];
    return {buf, toolbox::d2s_buffered_n(f, buf)};
}
} // namespace

using namespace toolbox;

BOOST_AUTO_TEST_SUITE(RyuSuite)

BOOST_AUTO_TEST_CASE(RyuCase)
{
    BOOST_TEST(to_string(0) == "0");
    BOOST_TEST(to_string(1) == "1");
    BOOST_TEST(to_string(-1) == "-1");
    BOOST_TEST(to_string(123) == "1.23E2");
    BOOST_TEST(to_string(-123) == "-1.23E2");
    BOOST_TEST(to_string(12.3) == "1.23E1");
    BOOST_TEST(to_string(-12.3) == "-1.23E1");
    BOOST_TEST(to_string(1.23) == "1.23");
    BOOST_TEST(to_string(-1.23) == "-1.23");
    BOOST_TEST(to_string(1.0 / 0.0) == "Infinity");
}

BOOST_AUTO_TEST_SUITE_END()
