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

#include <toolbox/util/Ryu.hpp>

#include <toolbox/bm.hpp>

#include <cstdio>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {
constexpr double Qty = 1e6;
constexpr double Price = 1.234567;

TOOLBOX_BENCHMARK(dtos_buf)
{
    char buf[64];
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            util::dtos(buf, Qty);
            util::dtos(buf, Price);
        }
    }
}

TOOLBOX_BENCHMARK(dtos_tls)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            util::dtos(Qty);
            util::dtos(Price);
        }
    }
}

TOOLBOX_BENCHMARK(dtofixed_buf)
{
    char buf[64];
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            util::dtofixed(buf, Qty);
            util::dtofixed(buf, Price);
        }
    }
}

TOOLBOX_BENCHMARK(dtofixed_tls)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            util::dtofixed(Qty);
            util::dtofixed(Price);
        }
    }
}

TOOLBOX_BENCHMARK(sprintf)
{
    char buf[64];
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            ::sprintf(buf, "%f", Qty);
            ::sprintf(buf, "%f", Price);
        }
    }
}

TOOLBOX_BENCHMARK(strfromd)
{
    char buf[64];
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            ::strfromd(buf, sizeof(buf), "%f", Qty);
            ::strfromd(buf, sizeof(buf), "%f", Price);
        }
    }
}

} // namespace
