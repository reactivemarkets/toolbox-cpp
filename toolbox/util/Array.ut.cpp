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

#include "Array.hpp"

#include <boost/test/unit_test.hpp>

#include <algorithm> // equal()
#include <numeric>   // accumulate()

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(ArraySuite)

BOOST_AUTO_TEST_CASE(ArraySizeCase)
{
    const int arr[] = {101, 202, 303};
    BOOST_TEST(array_size(arr) == 3);
}

BOOST_AUTO_TEST_CASE(ArrayViewCase)
{
    BOOST_TEST(!ArrayView<int>{}.data());
    BOOST_TEST(ArrayView<int>{}.empty());
    BOOST_TEST(ArrayView<int>{}.size() == 0U);

    const int arr[] = {101, 202, 303};
    ArrayView<int> av{arr};

    BOOST_TEST(av.data());
    BOOST_TEST(!av.empty());
    BOOST_TEST(av.size() == 3U);

    BOOST_TEST(av[0] == arr[0]);
    BOOST_TEST(av[1] == arr[1]);
    BOOST_TEST(av[2] == arr[2]);

    BOOST_TEST(av.front() == arr[0]);
    BOOST_TEST(av.back() == arr[2]);

    BOOST_TEST(equal(av.begin(), av.end(), arr));
    int rev[] = {303, 202, 101};

    BOOST_TEST(equal(av.rbegin(), av.rend(), rev));

    BOOST_TEST(make_array_view(arr, 2).size() == 2U);
    BOOST_TEST(make_array_view(arr).size() == 3U);
}

BOOST_AUTO_TEST_SUITE_END()
