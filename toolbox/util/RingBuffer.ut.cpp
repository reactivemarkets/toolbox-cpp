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

#include "RingBuffer.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(RingBufferSuite)

BOOST_AUTO_TEST_CASE(RingBufferCase)
{
    using IntRingBuffer = RingBuffer<int>;

    IntRingBuffer rb{4};
    BOOST_TEST(rb.empty());
    BOOST_TEST(!rb.full());
    BOOST_TEST(rb.size() == 0U);

    rb.write([](int& val) { val = 1; });
    BOOST_TEST(!rb.empty());
    BOOST_TEST(!rb.full());
    BOOST_TEST(rb.size() == 1U);

    rb.clear();
    BOOST_TEST(rb.empty());
    BOOST_TEST(!rb.full());
    BOOST_TEST(rb.size() == 0U);

    rb.push(1);
    rb.push(2);
    rb.push(3);
    rb.push(4);
    BOOST_TEST(!rb.empty());
    BOOST_TEST(rb.full());
    BOOST_TEST(rb.size() == 4U);

    rb.push(5);
    BOOST_TEST(!rb.empty());
    BOOST_TEST(rb.full());
    BOOST_TEST(rb.size() == 4U);

    int val;
    rb.fetch([&val](const int& ref) { val = ref; });
    BOOST_TEST(2);
    BOOST_TEST(!rb.empty());
    BOOST_TEST(!rb.full());
    BOOST_TEST(rb.size() == 3U);

    BOOST_TEST(rb.front() == 3);
    rb.pop();
    BOOST_TEST(rb.front() == 4);
    rb.pop();
    BOOST_TEST(rb.front() == 5);
    rb.pop();

    BOOST_TEST(rb.empty());
    BOOST_TEST(!rb.full());
    BOOST_TEST(rb.size() == 0U);
}

BOOST_AUTO_TEST_SUITE_END()
