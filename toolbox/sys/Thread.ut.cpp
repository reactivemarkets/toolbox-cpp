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

#include "Thread.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
uint32_t to_bitset(const cpu_set_t& cpuset) noexcept
{
    uint32_t bs{0};
    for (int i{0}; i < 32; ++i) {
        if (CPU_ISSET(i, &cpuset)) {
            bs |= (1 << i);
        }
    }
    return bs;
}
} // namespace

BOOST_AUTO_TEST_SUITE(ThreadSuite)

BOOST_AUTO_TEST_CASE(ThreadParseCpuSetCase)
{
    BOOST_TEST(to_bitset(parse_cpu_set(""sv)) == 0b0);
    BOOST_TEST(to_bitset(parse_cpu_set("0"sv)) == 0b1);
    BOOST_TEST(to_bitset(parse_cpu_set("1,2"sv)) == 0b110);
    BOOST_TEST(to_bitset(parse_cpu_set("0-3"sv)) == 0b1111);
    BOOST_TEST(to_bitset(parse_cpu_set("1,2,4-7"sv)) == 0b11110110);
    BOOST_TEST(to_bitset(parse_cpu_set("0-3,5,6,8-11"sv)) == 0b111101101111);
}

BOOST_AUTO_TEST_SUITE_END()
