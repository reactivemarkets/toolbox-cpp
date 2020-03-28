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

#include "Tuple.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(TupleSuite)

BOOST_AUTO_TEST_CASE(TupleApplyCase)
{
    auto tuple = std::make_tuple(32, 16.7, "foo");
    std::string res;
    auto fn = [&res](auto& v) {
        auto sstr = std::ostringstream();
        sstr << v;
        res = sstr.str();
    };
    auto expected = std::string{"32"};
    toolbox::tuple_apply<0>(tuple, fn);
    BOOST_TEST(res == expected);
    expected = std::string{"16.7"};
    toolbox::tuple_apply<1>(tuple, fn);
    BOOST_TEST(res == expected);
    expected = std::string{"foo"};
    toolbox::tuple_apply<2>(tuple, fn);
    BOOST_TEST(res == expected);
}

BOOST_AUTO_TEST_CASE(TupleForEachCase)
{
    auto sstr = std::ostringstream();
    auto tuple = std::make_tuple(123, 45.6, "foo", " ", "bar");
    toolbox::tuple_for_each(tuple, [&sstr](const auto& v) { sstr << v; });
    auto expected = std::string{"12345.6foo bar"};
    BOOST_TEST(sstr.str() == expected);
}
BOOST_AUTO_TEST_SUITE_END()
