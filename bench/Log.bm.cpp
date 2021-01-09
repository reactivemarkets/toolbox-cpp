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

#include <toolbox/sys/Log.hpp>

#include <toolbox/bm.hpp>

#include <cstring>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {
namespace noformat {
// Specific Log operator<< to allow non formatted writing.
Log& operator<<(Log& log, std::string_view str)
{
    return log(str.data(), str.size());
}
} // namespace noformat

auto prev_level = set_log_level(Log::Info);
auto prev_logger = set_logger(null_logger);

TOOLBOX_BENCHMARK(log_formated)
{
    while (ctx) {
        for (auto _ : ctx.range(5000)) {
            TOOLBOX_LOG(Log::Info) << "BenchmarkString"sv;
        }
    }
}

TOOLBOX_BENCHMARK(log_unformated)
{
    while (ctx) {
        for (auto _ : ctx.range(5000)) {
            using namespace noformat;
            TOOLBOX_LOG(Log::Info) << "BenchmarkString"sv;
        }
    }
}

} // namespace
