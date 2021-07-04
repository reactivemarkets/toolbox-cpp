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

#include <toolbox/util/Utility.hpp>

#include <toolbox/bm.hpp>

#include <cmath>
#include <cstdio>

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
        for (auto _ : ctx.range(1000)) {
            auto x = std::to_string(DoubleValue);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(to_string_int)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            auto x = std::to_string(IntValue);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_lib)
{
    while (ctx) {
        for (auto i : ctx.range(10000)) {
            auto x = util::dec_digits(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_div)
{
    while (ctx) {
        for (auto i : ctx.range(10000)) {
            auto x = ::dec_digits_div(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_math)
{
    while (ctx) {
        for (auto i : ctx.range(10000)) {
            auto x = ::dec_digits_math(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_stdio)
{
    while (ctx) {
        for (auto i : ctx.range(10000)) {
            auto x = ::dec_digits_stdio(i);
            bm::do_not_optimise(x);
        }
    }
}

TOOLBOX_BENCHMARK(dec_digits_less)
{
    while (ctx) {
        for (auto i : ctx.range(10000)) {
            auto x = ::dec_digits_less(i);
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
        for (unsigned i : ctx.range(1000)) {
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
        for (unsigned i : ctx.range(1000)) {
            int sum{0};
            for (const auto sv : BoolArray) {
                sum += static_cast<int>(::stob_string(sv));
            }
            bm::do_not_optimise(sum);
        }
    }
}

} // namespace
