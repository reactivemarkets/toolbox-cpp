// The Reactive C++ Toolbox.
// Copyright (C) 2025 Reactive Markets Limited
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

#include <toolbox/bm.hpp>
#include <toolbox/bm/Utility.hpp>
#include <toolbox/util/Stream.hpp>

#include <sstream>
#include <iomanip>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

TOOLBOX_BENCHMARK(std_io_manipulator)
{
    stringstream ss;
    while (ctx) {
        ss.str(string{}); // reset it
        for ([[maybe_unused]] auto i : ctx.range(100)) {
            ss << setw(20) << fixed << setprecision(6) << double{1.2345678} << '\n';
            ss << setw(10) << setfill('0') << 1234 << '\n';
            ss << setw(10) << setfill('*') << 54 << '\n';
            bm::do_not_optimise(ss);
        }
    }
}

TOOLBOX_BENCHMARK(std_format_manipulator)
{
    stringstream ss;
    while (ctx) {
        ss.str(string{}); // reset it
        for ([[maybe_unused]] auto i : ctx.range(100)) {
            ss << std::format("{:20.6f}", double{1.2345678}) << '\n';
            ss << std::format("{:0>10}", int{1234}) << '\n';
            ss << std::format("{:*>10}", 54) << '\n';
            bm::do_not_optimise(ss);
        }
    }
}

} // namespace
