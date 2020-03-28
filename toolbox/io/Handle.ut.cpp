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

#include "Handle.hpp"

#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace std;
using namespace toolbox;

namespace {
int last_closed{};

struct TestPolicy {
    using Id = int;
    static constexpr int invalid() noexcept { return -1; }
    static void close(int id) noexcept { last_closed = id; }
};

using TestHandle = BasicHandle<TestPolicy>;

ostream& operator<<(ostream& os, const TestHandle& h)
{
    return os << h.get();
}

} // namespace

BOOST_AUTO_TEST_SUITE(HandleSuite)

BOOST_AUTO_TEST_CASE(HandleInvalidCase)
{
    last_closed = 0;

    BOOST_TEST(!TestHandle{});
    BOOST_TEST(TestHandle{}.get() == -1);
    BOOST_TEST(last_closed == 0);

    BOOST_TEST(!TestHandle{nullptr});
    BOOST_TEST(TestHandle{nullptr}.get() == -1);
    BOOST_TEST(last_closed == 0);
}

BOOST_AUTO_TEST_CASE(HandleCloseCase)
{
    last_closed = 0;

    BOOST_TEST(TestHandle{1});
    BOOST_TEST(TestHandle{1}.get() == 1);
    BOOST_TEST(last_closed == 1);
}

BOOST_AUTO_TEST_CASE(HandleReleaseCase)
{
    last_closed = 0;

    BOOST_TEST(TestHandle{1}.release() == 1);
    BOOST_TEST(last_closed == 0);
}

BOOST_AUTO_TEST_CASE(HandleResetCase)
{
    last_closed = 0;
    {
        TestHandle h{1};
        h.reset(2);
        BOOST_TEST(last_closed == 1);
    }
    BOOST_TEST(last_closed == 2);
}

BOOST_AUTO_TEST_CASE(HandleSwapCase)
{
    last_closed = 0;
    {
        TestHandle h{1};
        TestHandle{2}.swap(h);
        BOOST_TEST(last_closed == 1);
    }
    BOOST_TEST(last_closed == 2);
}

BOOST_AUTO_TEST_CASE(HandleMoveCase)
{
    last_closed = 0;

    TestHandle h{1};
    TestHandle{move(h)};
    BOOST_TEST(last_closed == 1);
    BOOST_TEST(h.get() == -1);

    h.reset(2);
    {
        TestHandle tmp;
        tmp = move(h);
    }
    BOOST_TEST(last_closed == 2);
    BOOST_TEST(h.get() == -1);
}

BOOST_AUTO_TEST_CASE(HandleEqualityCase)
{
    BOOST_TEST(TestHandle{1} == TestHandle{1});
    BOOST_TEST(TestHandle{1} != TestHandle{2});
}

BOOST_AUTO_TEST_SUITE_END()
