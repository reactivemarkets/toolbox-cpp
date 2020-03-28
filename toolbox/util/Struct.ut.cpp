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

#include "Struct.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
namespace tag {
struct Foo {
};
struct Bar {
};
} // namespace tag

static_assert(empty(Struct));
static_assert(!empty(Struct.extend<tag::Foo>(101)));
static_assert(size(Struct.extend<tag::Foo>(101)) == 1);
static_assert(has<tag::Foo>(Struct.extend<tag::Foo>(101)));
static_assert(!has<tag::Bar>(Struct.extend<tag::Foo>(101)));
static_assert(get<tag::Foo>(Struct.extend<tag::Foo>(101)) == 101);

} // namespace

BOOST_AUTO_TEST_SUITE(StructSuite)

BOOST_AUTO_TEST_CASE(StructEmptyCase)
{
    const auto s = Struct;
    BOOST_TEST(empty(s));
    BOOST_TEST(size(s) == 0U);
    BOOST_TEST(!has<tag::Foo>(s));
    BOOST_TEST(!has<tag::Bar>(s));
}

BOOST_AUTO_TEST_CASE(StructFooCase)
{
    struct Test {
        int x;
    };

    const auto s = Struct.extend<tag::Foo>(Test{101});
    BOOST_TEST(!empty(s));
    BOOST_TEST(size(s) == 1U);
    BOOST_TEST(has<tag::Foo>(s));
    BOOST_TEST(!has<tag::Bar>(s));
    BOOST_TEST(get<tag::Foo>(s).x == 101);
}

BOOST_AUTO_TEST_CASE(StructFooBarCase)
{
    struct Test {
        int x;
    };

    const auto s = Struct.extend<tag::Foo>(Test{101}).extend<tag::Bar>(Test{202});
    BOOST_TEST(!empty(s));
    BOOST_TEST(size(s) == 2U);
    BOOST_TEST(has<tag::Foo>(s));
    BOOST_TEST(has<tag::Bar>(s));
    BOOST_TEST(get<tag::Foo>(s).x == 101);
    BOOST_TEST(get<tag::Bar>(s).x == 202);
}

BOOST_AUTO_TEST_CASE(StructLambdaCase)
{
    const auto s = Struct.extend<tag::Foo>([](int x) { return x + x; });
    BOOST_TEST(get<tag::Foo>(s)(101) == 202);
}

BOOST_AUTO_TEST_CASE(StructMutableCase)
{
    auto s = Struct.extend<tag::Foo>(101);
    BOOST_TEST(get<tag::Foo>(s) == 101);
    get<tag::Foo>(s) = 202;
    BOOST_TEST(get<tag::Foo>(s) == 202);
}

BOOST_AUTO_TEST_SUITE_END()
