// The Reactive C++ Toolbox.
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

#include "RateLimit.hpp"

#include <toolbox/util/String.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(RateLimitSuite)

BOOST_AUTO_TEST_CASE(RateLimitCase)
{
    BOOST_TEST(to_string(RateLimit(1, 5s)) == "1/5");
    const auto rl = from_string<RateLimit>("3/5");
    BOOST_TEST(rl.limit() == 3);
    BOOST_TEST(rl.interval().count() == 5);
}

BOOST_AUTO_TEST_CASE(RateWindowCase)
{
    const auto t = MonoClock::now();

    RateWindow rw{1s};
    BOOST_TEST(rw.count() == 0);

    rw.add(t + 0s, 1);
    BOOST_TEST(rw.count() == 1);
    rw.add(t + 0s, 2);
    BOOST_TEST(rw.count() == 3);
    rw.add(t + 1s, 1);
    BOOST_TEST(rw.count() == 1);
    rw.add(t + 2s, 2);
    BOOST_TEST(rw.count() == 2);
    rw.add(t + 3s, 3);
    BOOST_TEST(rw.count() == 3);

    rw = RateWindow{5s};
    BOOST_TEST(rw.count() == 0);

    rw.add(t + 0s, 1);
    BOOST_TEST(rw.count() == 1);
    rw.add(t + 1s, 2);
    BOOST_TEST(rw.count() == 3);
    rw.add(t + 2s, 3);
    BOOST_TEST(rw.count() == 6);
    rw.add(t + 3s, 4);
    BOOST_TEST(rw.count() == 10);
    rw.add(t + 4s, 5);
    BOOST_TEST(rw.count() == 15);

    // Circular buffer wraps and replaces first entry.
    rw.add(t + 5s, 6);
    BOOST_TEST(rw.count() == 20);

    // Skip two entries entirely.
    rw.add(t + 8s, 7);
    BOOST_TEST(rw.count() == 18);

    // Skip every entry in the buffer.
    rw.add(t + 13s, 8);
    BOOST_TEST(rw.count() == 8);
}

BOOST_AUTO_TEST_SUITE_END()
