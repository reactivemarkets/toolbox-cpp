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

#include "Time.hpp"

#include <toolbox/util/Benchmark.hpp>

#include <sys/syscall.h>

using namespace std;
using namespace toolbox;

TOOLBOX_BENCHMARK("std/clock_gettime_realtime", []() {
    timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);
});

TOOLBOX_BENCHMARK("std/sys_clock_gettime_realtime", []() {
    timespec ts;
    syscall(SYS_clock_gettime, CLOCK_REALTIME, &ts);
});

TOOLBOX_BENCHMARK("std/clock_gettime_monotonic", []() {
    timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
});

TOOLBOX_BENCHMARK("std/sys_clock_gettime_monotonic", []() {
    timespec ts;
    syscall(SYS_clock_gettime, CLOCK_MONOTONIC, &ts);
});

TOOLBOX_BENCHMARK("time/cycl_time_now", []() { CyclTime::now(); });
TOOLBOX_BENCHMARK("time/cycl_time_mono_time", []() {
    auto now = CyclTime::now();
    clobber_memory();
    auto t = now.mono_time();
    do_not_optimise(t);
});
