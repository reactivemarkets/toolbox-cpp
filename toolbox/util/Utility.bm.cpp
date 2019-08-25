// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#include "Benchmark.hpp"

#include <cmath>

using namespace std;
using namespace toolbox;

namespace {

int dec_digits2(int64_t i) noexcept __attribute__((noinline));
int dec_digits2(int64_t i) noexcept
{
    int count = 0;
    do {
        i = i / 10;
        ++count;
    } while (i != 0);
    return count;
}

int dec_digits3(int64_t i) noexcept __attribute__((noinline));
int dec_digits3(int64_t i) noexcept
{
    return i ? floor(log10(i) + 1) : 1;
}

int dec_digits4(int64_t i) noexcept __attribute__((noinline));
int dec_digits4(int64_t i) noexcept
{
    return snprintf(nullptr, 0, "%lld", static_cast<long long>(i));
}

int dec_digits5(int64_t i) noexcept __attribute__((noinline));
int dec_digits5(int64_t i) noexcept
{
    if (i < 10)
        return 1;
    if (i < 100)
        return 2;
    if (i < 1000)
        return 3;
    if (i < 10000)
        return 4;
    if (i < 100000)
        return 5;
    if (i < 1000000)
        return 6;
    if (i < 10000000)
        return 7;
    if (i < 100000000)
        return 8;
    if (i < 1000000000)
        return 9;
    if (i < 10000000000)
        return 10;
    if (i < 100000000000)
        return 11;
    if (i < 1000000000000)
        return 12;
    if (i < 10000000000000)
        return 13;
    if (i < 100000000000000)
        return 14;
    if (i < 1000000000000000)
        return 15;
    if (i < 10000000000000000)
        return 16;
    if (i < 100000000000000000)
        return 17;
    if (i < 1000000000000000000)
        return 18;
    return 19;
}

int hex_digits2(int64_t i) noexcept __attribute__((noinline));
int hex_digits2(int64_t i) noexcept
{
    int n{0};
    if (i & 0xffffffff00000000) {
        n += 8;
        i >>= 32;
    }
    if (i & 0xffff0000) {
        n += 4;
        i >>= 16;
    }
    if (i & 0xff00) {
        n += 2;
        i >>= 8;
    }
    if (i & 0xf0) {
        n += 2;
    } else if (i & 0x0f) {
        ++n;
    }
    return n;
}

} // namespace

TOOLBOX_BENCHMARK("std/double_to_string", [](double x) {
    return std::to_string(x);
}).range(0.0, 10000.0);
TOOLBOX_BENCHMARK("std/int_to_string", [](int x) { return std::to_string(x); }).range(0, 10000);

TOOLBOX_BENCHMARK("std/isdigit", [](int64_t i) { return std::isdigit(i); }).range(0, 1000);
TOOLBOX_BENCHMARK("utility/isdigit", [](int64_t i) {
    return toolbox::util::isdigit(i);
}).range(0, 1000);

TOOLBOX_BENCHMARK("utility/dec_digits1", dec_digits).range(0, 400000);
TOOLBOX_BENCHMARK("utility/dec_digits2", dec_digits2).range(0, 400000);
TOOLBOX_BENCHMARK("utility/dec_digits3", dec_digits3).range(0, 400000);
TOOLBOX_BENCHMARK("utility/dec_digits4", dec_digits4).range(0, 400000);
TOOLBOX_BENCHMARK("utility/dec_digits5", dec_digits5).range(0, 400000);
TOOLBOX_BENCHMARK("utility/dec_digits5", dec_digits5).range(0, 400000);

TOOLBOX_BENCHMARK("utility/hex_digits1", hex_digits).range(0, 400000);
TOOLBOX_BENCHMARK("utility/hex_digits2", hex_digits2).range(0, 400000);
