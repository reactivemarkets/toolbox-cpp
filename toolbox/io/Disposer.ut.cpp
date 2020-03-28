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

#include "Disposer.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
struct Disposer : BasicDisposer<Disposer> {
    using BasicDisposer<Disposer>::is_locked;
    using BasicDisposer<Disposer>::lock_this;
    void dispose_now(CyclTime now) noexcept
    {
        // Ensure that dispose_now can safely call dispose.
        // This could happen if dispose_now invoked a callback.
        dispose(now);
        ++disposed;
    }
    int disposed{0};
};
} // namespace

BOOST_AUTO_TEST_SUITE(DisposerSuite)

BOOST_AUTO_TEST_CASE(DisposerCase)
{
    const auto now = CyclTime::now();
    Disposer d;
    BOOST_TEST(!d.is_locked());
    BOOST_TEST(d.disposed == 0);
    {
        auto lock = d.lock_this(now);
        BOOST_TEST(d.is_locked());
    }
    BOOST_TEST(!d.is_locked());
    BOOST_TEST(d.disposed == 0);
    {
        auto outer_lock = d.lock_this(now);
        BOOST_TEST(d.is_locked());
        {
            auto inner_lock = d.lock_this(now);
            d.dispose(now);
            BOOST_TEST(d.is_locked());
            BOOST_TEST(d.disposed == 0);
        }
        // And again.
        d.dispose(now);
        BOOST_TEST(d.is_locked());
        BOOST_TEST(d.disposed == 0);
    }
    BOOST_TEST(d.disposed == 1);
}

BOOST_AUTO_TEST_SUITE_END()
