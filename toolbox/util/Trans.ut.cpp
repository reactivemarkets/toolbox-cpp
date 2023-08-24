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

#include "Trans.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

struct Foo {

    int start_calls{0};
    int commit_calls{0};
    int rollback_calls{0};

    void clear() noexcept
    {
        start_calls = 0;
        commit_calls = 0;
        rollback_calls = 0;
    }

    void start() { ++start_calls; }
    void commit() { ++commit_calls; }
    void rollback() noexcept { ++rollback_calls; }
};

using Trans = BasicTrans<Foo>;

} // namespace

BOOST_AUTO_TEST_SUITE(TransSuite)

BOOST_AUTO_TEST_CASE(TransScopedCommitCase)
{
    Foo foo;
    {
        Trans trans{foo};
        trans.commit();
    }
    BOOST_CHECK_EQUAL(foo.start_calls, 1);
    BOOST_CHECK_EQUAL(foo.commit_calls, 1);
    BOOST_CHECK_EQUAL(foo.rollback_calls, 0);
    {
        Trans trans{foo};
        trans.commit();
    }
    BOOST_CHECK_EQUAL(foo.start_calls, 2);
    BOOST_CHECK_EQUAL(foo.commit_calls, 2);
    BOOST_CHECK_EQUAL(foo.rollback_calls, 0);
}

BOOST_AUTO_TEST_CASE(TransScopedRollbackCase)
{
    Foo foo;
    {
        Trans trans{foo};
    }
    BOOST_CHECK_EQUAL(foo.start_calls, 1);
    BOOST_CHECK_EQUAL(foo.commit_calls, 0);
    BOOST_CHECK_EQUAL(foo.rollback_calls, 1);
    {
        Trans trans{foo};
    }
    BOOST_CHECK_EQUAL(foo.start_calls, 2);
    BOOST_CHECK_EQUAL(foo.commit_calls, 0);
    BOOST_CHECK_EQUAL(foo.rollback_calls, 2);
}

BOOST_AUTO_TEST_SUITE_END()
