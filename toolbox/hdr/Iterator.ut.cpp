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

#include "Utility.hpp"

#include "Histogram.hpp"
#include "Iterator.hpp"

#include <boost/test/unit_test.hpp>

#include <cmath>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(IteratorSuite)

BOOST_AUTO_TEST_CASE(IteratorLinearIteratorCase)
{
    HdrHistogram h{1, 255, 2};
    BOOST_TEST(h.record_value(193));
    BOOST_TEST(h.record_value(255));
    BOOST_TEST(h.record_value(0));
    BOOST_TEST(h.record_value(1));
    BOOST_TEST(h.record_value(64));
    BOOST_TEST(h.record_value(128));

    int step_count{0};
    int64_t total_count{0};

    HdrLinearIterator iter{h, 64};
    while (iter.next()) {
        total_count += iter.count_added_in_this_iteration_step();
        if (step_count == 0) {
            h.record_value(2);
        }
        ++step_count;
    }

    BOOST_TEST(step_count == 4);
    BOOST_TEST(total_count == 6);
}

BOOST_AUTO_TEST_SUITE_END()
