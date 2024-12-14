// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2024 Reactive Markets Limited
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

#include "Random.hpp"

#include <boost/test/tools/old/interface.hpp>
#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(RandomSuite)

BOOST_AUTO_TEST_CASE(RandIntCase)
{
    constexpr int lower_bound = 0;
    constexpr int upper_bound = 100;

    vector<int> rand_ints;
    for (int i = 0; i < 100; i++) {
        rand_ints.push_back(randint(lower_bound, upper_bound));
        std::cout << rand_ints.back() << '\n';
    }

    for (int num : rand_ints) {
        BOOST_CHECK_GE(num, lower_bound);
        BOOST_CHECK_LE(num, upper_bound);
    }
}

BOOST_AUTO_TEST_CASE(RandIntNegativeCase)
{
    constexpr int lower_bound = -100;
    constexpr int upper_bound = 100;

    vector<int> rand_ints;
    for (int i = 0; i < 100; i++) {
        rand_ints.push_back(randint(lower_bound, upper_bound));
        std::cout << rand_ints.back() << '\n';
    }

    for (int num : rand_ints) {
        BOOST_CHECK_GE(num, lower_bound);
        BOOST_CHECK_LE(num, upper_bound);
    }
}

BOOST_AUTO_TEST_SUITE_END()
