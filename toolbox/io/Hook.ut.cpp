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

#include "Hook.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

// This class template implements a hook that will call a function object before unhooking itself.
template <typename FnT>
struct Test {
    explicit Test(FnT fn)
    : fn_{fn}
    , hook{bind<&Test::call_then_delete>(this)}
    {
    }
    void call_then_delete(CyclTime /*now*/)
    {
        fn_();
        delete this;
    }
    FnT fn_;
    Hook hook;
};

template <typename FnT>
auto make_test(FnT fn)
{
    return new Test{fn};
}

} // namespace

BOOST_AUTO_TEST_SUITE(HookSuite)

BOOST_AUTO_TEST_CASE(HookAutoUnlinkCase)
{
    HookList l;
    BOOST_CHECK(l.empty());
    {
        Hook h;
        BOOST_CHECK(!h.is_linked());
        l.push_back(h);
        BOOST_CHECK(h.is_linked());
        BOOST_CHECK(!l.empty());
    }
    BOOST_CHECK(l.empty());
}

BOOST_AUTO_TEST_CASE(HookDispatchCase)
{
    int i{0};
    auto fn = [&i](CyclTime) { ++i; };

    HookList l;
    {
        Hook h1{bind(&fn)};
        l.push_back(h1);

        dispatch(CyclTime::now(), l);
        BOOST_CHECK_EQUAL(i, 1);

        dispatch(CyclTime::now(), l);
        BOOST_CHECK_EQUAL(i, 2);
        {
            Hook h2{bind(&fn)};
            l.push_back(h2);

            dispatch(CyclTime::now(), l);
            BOOST_CHECK_EQUAL(i, 4);
        }
    }
}

BOOST_AUTO_TEST_CASE(HookReentrantCase)
{
    int i{0};
    auto fn = [&i](CyclTime) { ++i; };

    HookList l;

    Hook h1{bind(&fn)};
    l.push_back(make_test([&l, &h1]() { l.push_back(h1); })->hook);

    Hook h2{bind(&fn)};
    l.push_back(make_test([&l, &h2]() { l.push_back(h2); })->hook);

    BOOST_CHECK_EQUAL(l.size(), 2);
    BOOST_CHECK(!h1.is_linked());
    BOOST_CHECK(!h2.is_linked());

    dispatch(CyclTime::now(), l);
    BOOST_CHECK_EQUAL(i, 2);

    BOOST_CHECK_EQUAL(l.size(), 2);
    BOOST_CHECK(h1.is_linked());
    BOOST_CHECK(h2.is_linked());

    dispatch(CyclTime::now(), l);
    BOOST_CHECK_EQUAL(i, 4);
}

BOOST_AUTO_TEST_SUITE_END()
