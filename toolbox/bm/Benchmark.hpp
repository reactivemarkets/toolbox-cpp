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

#ifndef TOOLBOX_BM_BENCHMARK_HPP
#define TOOLBOX_BM_BENCHMARK_HPP

#include <toolbox/Config.h>

namespace toolbox::bm {
class BenchmarkCtx;

struct TOOLBOX_API Benchmark {
    Benchmark(const char* name, void (*fn)(BenchmarkCtx&));
    const char* const name;
    void (*fn)(BenchmarkCtx&);
};

namespace detail {
TOOLBOX_API int main(int argc, char* argv[]);
} // namespace detail
} // namespace toolbox::bm

#define TOOLBOX_BENCHMARK(NAME)                                                                    \
    namespace benchmark::NAME {                                                                    \
    void fn(::toolbox::bm::BenchmarkCtx& ctx);                                                     \
    ::toolbox::bm::Benchmark bm{#NAME, fn};                                                        \
    }                                                                                              \
    void benchmark::NAME::fn(toolbox::bm::BenchmarkCtx& ctx)

#define TOOLBOX_BENCHMARK_MAIN                                                                     \
    int main(int argc, char* argv[]) { return toolbox::bm::detail::main(argc, argv); }

#endif // TOOLBOX_BM_BENCHMARK_HPP
