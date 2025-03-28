// The Reactive C++ Toolbox.
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

#include <toolbox/util/Random.hpp>
#include <toolbox/util/Utility.hpp>
#include <toolbox/util/Math.hpp>

#include <toolbox/bm.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <vector>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {
constexpr double DoubleValue = -12345678.12345678;
constexpr int IntValue = -12345678;
constexpr array BoolArray{
    ""sv,     "0"sv,    "F"sv,     "N"sv,     "f"sv,     "n"sv,   "1"sv,   "T"sv,
    "Y"sv,    "t"sv,    "y"sv,     "NO"sv,    "No"sv,    "no"sv,  "ON"sv,  "On"sv,
    "on"sv,   "OFF"sv,  "Off"sv,   "off"sv,   "YES"sv,   "Yes"sv, "yes"sv, "TRUE"sv,
    "True"sv, "true"sv, "FALSE"sv, "False"sv, "false"sv,
};

// K must be within the range [0,19]
std::vector<std::int64_t> generate_mixed_digit_nums(std::int64_t N, int K) noexcept {
    std::vector<std::int64_t> res;
    res.reserve(N);

    for(std::int64_t i = 0; i < N; i++) {
        int num_of_digits = randint(1, K);

        // e.g. if num_of_digits=3
        // lr := 10^(3-1) = 100
        // ur := 10^3 - 1 = 999
        std::int64_t lr = util::pow10(num_of_digits-1);
        std::int64_t ur = util::pow10(num_of_digits) - 1u;

        res.push_back(randint(lr, ur));
    }

    return res;
}

const std::vector random_digit_nums = generate_mixed_digit_nums(1'000'000, 7);

int dec_digits_branch(std::int64_t i) noexcept __attribute__((noinline));
int dec_digits_branch(std::int64_t i) noexcept
{
    return i < 10000000000        ? i < 100000 ? i < 100 ? i < 10 ? 1 : 2
                       : i < 1000                        ? 3
                       : i < 10000                       ? 4
                                                         : 5
                   : i < 10000000              ? i < 1000000 ? 6 : 7
                   : i < 100000000             ? 8
                   : i < 1000000000            ? 9
                                               : 10
        : i < 1000000000000000    ? i < 1000000000000 ? i < 100000000000 ? 11 : 12
               : i < 10000000000000                   ? 13
               : i < 100000000000000                  ? 14
                                                      : 15
        : i < 100000000000000000  ? i < 10000000000000000 ? 16 : 17
        : i < 1000000000000000000 ? 18
                                  : 19;
}

int dec_digits_div(int64_t i) noexcept __attribute__((noinline));
int dec_digits_div(int64_t i) noexcept
{
    int count = 0;
    do {
        i = i / 10;
        ++count;
    } while (i != 0);
    return count;
}

int dec_digits_math(int64_t i) noexcept __attribute__((noinline));
int dec_digits_math(int64_t i) noexcept
{
    return i ? floor(log10(i) + 1) : 1;
}

int dec_digits_stdio(int64_t i) noexcept __attribute__((noinline));
int dec_digits_stdio(int64_t i) noexcept
{
    return snprintf(nullptr, 0, "%lld", static_cast<long long>(i));
}

int dec_digits_less(int64_t i) noexcept __attribute__((noinline));
int dec_digits_less(int64_t i) noexcept
{
    if (i < 10) {
        return 1;
    }
    if (i < 100) {
        return 2;
    }
    if (i < 1000) {
        return 3;
    }
    if (i < 10000) {
        return 4;
    }
    if (i < 100000) {
        return 5;
    }
    if (i < 1000000) {
        return 6;
    }
    if (i < 10000000) {
        return 7;
    }
    if (i < 100000000) {
        return 8;
    }
    if (i < 1000000000) {
        return 9;
    }
    if (i < 10000000000) {
        return 10;
    }
    if (i < 100000000000) {
        return 11;
    }
    if (i < 1000000000000) {
        return 12;
    }
    if (i < 10000000000000) {
        return 13;
    }
    if (i < 100000000000000) {
        return 14;
    }
    if (i < 1000000000000000) {
        return 15;
    }
    if (i < 10000000000000000) {
        return 16;
    }
    if (i < 100000000000000000) {
        return 17;
    }
    if (i < 1000000000000000000) {
        return 18;
    }
    return 19;
}

int hex_digits_bitshift(int64_t i) noexcept
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

bool stob_string(string_view sv, bool dfl = false) noexcept __attribute__((noinline));
bool stob_string(string_view sv, bool dfl) noexcept
{
    string s;
    s.reserve(sv.size());
    transform(sv.begin(), sv.end(), back_inserter(s), [](char c) { return toupper(c); });

    bool val{dfl};
    switch (s.size()) {
    case 1:
        switch (s[0]) {
        case '0':
        case 'F':
        case 'N':
            val = false;
            break;
        case '1':
        case 'T':
        case 'Y':
            val = true;
            break;
        }
        break;
    case 2:
        if (s == "NO") {
            val = false;
        } else if (s == "ON") {
            val = true;
        }
        break;
    case 3:
        if (s == "OFF") {
            val = false;
        } else if (s == "YES") {
            val = true;
        }
        break;
    case 4:
        if (s == "TRUE") {
            val = true;
        }
        break;
    case 5:
        if (s == "FALSE") {
            val = false;
        }
        break;
    }
    return val;
}

TOOLBOX_BENCHMARK(to_string_double)
{
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(1000)) {
            auto x = std::to_string(DoubleValue);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(to_string_int)
{
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(1000)) {
            auto x = std::to_string(IntValue);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_lib_unsigned)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto rnum = static_cast<uint64_t>(random_digit_nums[i]);
            auto x = util::dec_digits(rnum);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_lib_signed)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = util::dec_digits(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_branch)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = ::dec_digits_branch(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_div)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = ::dec_digits_div(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_math)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = ::dec_digits_math(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_stdio)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = ::dec_digits_stdio(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_less)
{
    while (ctx) {
        for (auto i : ctx.range(random_digit_nums.size())) {
            auto x = ::dec_digits_less(random_digit_nums[i]);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(hex_digits)
{
    while (ctx) {
        for (unsigned i : ctx.range(10000)) {
            auto x = util::hex_digits(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(hex_digits_bitshift)
{
    while (ctx) {
        for (unsigned i : ctx.range(10000)) {
            auto x = ::hex_digits_bitshift(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(stob)
{
    while (ctx) {
        for ([[maybe_unused]] unsigned _ : ctx.range(1000)) {
            int sum{0};
            for (const auto sv : BoolArray) {
                sum += static_cast<int>(toolbox::stob(sv));
            }
            bm::do_not_optimise(sum);
        }
    }
}

TOOLBOX_BENCHMARK(stob_string)
{
    while (ctx) {
        for ([[maybe_unused]] unsigned _ : ctx.range(1000)) {
            int sum{0};
            for (const auto sv : BoolArray) {
                sum += static_cast<int>(::stob_string(sv));
            }
            bm::do_not_optimise(sum);
        }
    }
}

} // namespace
