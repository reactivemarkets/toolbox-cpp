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

#ifndef TOOLBOX_BM_SUITE_HPP
#define TOOLBOX_BM_SUITE_HPP

#include <toolbox/bm/Ctx.hpp>

#include <toolbox/hdr/Histogram.hpp>

namespace toolbox::bm {

class TOOLBOX_API BenchmarkSuite {
  public:
    explicit BenchmarkSuite(std::ostream& os, double value_scale = 1.0);

    template <typename FnT>
    void run(const char* name, FnT fn)
    {
        using namespace std::literals::chrono_literals;
        constexpr auto duration = 3s;

        HdrHistogram hist{1, 1'000'000'000, 5};
        BenchmarkCtx ctx{hist};
        Alarm alarm{duration, [&ctx]() { ctx.stop(); }};
        fn(ctx);
        report(name, hist);
    }
    void report(const char* name, HdrHistogram& hist);

  private:
    std::ostream& os_;
    double value_scale_;
};

} // namespace toolbox::bm

#endif // TOOLBOX_BM_SUITE_HPP
