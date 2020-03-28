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

#include <toolbox/sys/Time.hpp>

#include <toolbox/bm.hpp>

#include <sys/syscall.h>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

TOOLBOX_BENCHMARK(clock_gettime_realtime)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            timespec ts;
            ::clock_gettime(CLOCK_REALTIME, &ts);
        }
    }
}

TOOLBOX_BENCHMARK(sys_clock_gettime_realtime)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            timespec ts;
            syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
        }
    }
}

TOOLBOX_BENCHMARK(clock_gettime_monotonic)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            timespec ts;
            ::clock_gettime(CLOCK_MONOTONIC, &ts);
        }
    }
}

TOOLBOX_BENCHMARK(sys_clock_gettime_monotonic)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            timespec ts;
            syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);
        }
    }
}

TOOLBOX_BENCHMARK(cycl_time_now)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            CyclTime::now();
        }
    }
}

TOOLBOX_BENCHMARK(cycl_time_mono_time)
{
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            auto now = CyclTime::now();
            bm::clobber_memory();
            auto t = now.mono_time();
            bm::do_not_optimise(t);
        }
    }
}

} // namespace
