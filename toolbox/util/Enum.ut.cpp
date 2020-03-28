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

#include "Enum.hpp"

#include <boost/test/unit_test.hpp>

#include <boost/lexical_cast.hpp>

using namespace boost;
using namespace std;
using namespace toolbox;

namespace toolbox {
inline namespace util {

enum class Test : int { Foo = 1, Bar = 2, Baz = 4, Qux = 8 };

constexpr Test operator""_test(unsigned long long val) noexcept
{
    return box<Test>(val);
}

const char* enum_string(Test t)
{
    switch (t) {
    case Test::Foo:
        return "Foo";
    case Test::Bar:
        return "Bar";
    case Test::Baz:
        return "Baz";
    case Test::Qux:
        return "Qux";
    }
    terminate();
}

inline ostream& operator<<(ostream& os, Test t)
{
    return os << enum_string(t);
}

} // namespace util
} // namespace toolbox

BOOST_AUTO_TEST_SUITE(EnumSuite)

BOOST_AUTO_TEST_CASE(EnumStringCase)
{
    BOOST_TEST(lexical_cast<string>(Test::Foo) == "Foo");
    BOOST_TEST(lexical_cast<string>(Test::Bar) == "Bar");
    BOOST_TEST(lexical_cast<string>(Test::Baz) == "Baz");
    BOOST_TEST(lexical_cast<string>(Test::Qux) == "Qux");
}

BOOST_AUTO_TEST_SUITE_END()
