// The Reactive C++ Toolbox.
// Copyright (C) 2023 Reactive Markets Limited
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

#include <toolbox/hdr/Histogram.hpp>

#include <toolbox/bm.hpp>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

void histogram_new(bm::Context& ctx, std::int64_t lowest_trackable_value,
                   std::int64_t highest_trackable_value, int significant_figures)
{
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(10)) {
            std::unique_ptr<Histogram> hist{new Histogram{
                lowest_trackable_value, highest_trackable_value, significant_figures}};
            bm::do_not_optimise(hist.get());
        }
    }
}

void histogram_reset(bm::Context& ctx, std::int64_t lowest_trackable_value,
                     std::int64_t highest_trackable_value, int significant_figures)
{
    std::unique_ptr<Histogram> hist{
        new Histogram{lowest_trackable_value, highest_trackable_value, significant_figures}};
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(10)) {
            hist->reset();
        }
    }
}

TOOLBOX_BENCHMARK(histogram_new_1e6_3sf)
{
    // Record microseconds with 3sf and max expected value of one second.
    histogram_new(ctx, 1, 1'000'000, 3);
}

TOOLBOX_BENCHMARK(histogram_new_1e7_3sf)
{
    // Record microseconds with 3sf and max expected value of ten seconds.
    histogram_new(ctx, 1, 10'000'000, 3);
}

TOOLBOX_BENCHMARK(histogram_new_1e6_5sf)
{
    // Record microseconds with 5sf and max expected value of one second.
    histogram_new(ctx, 1, 1'000'000, 5);
}

TOOLBOX_BENCHMARK(histogram_new_1e7_5sf)
{
    // Record microseconds with 5sf and max expected value of ten seconds.
    histogram_new(ctx, 1, 10'000'000, 5);
}

TOOLBOX_BENCHMARK(histogram_reset_1e6_3sf)
{
    // Record microseconds with 3sf and max expected value of one second.
    histogram_reset(ctx, 1, 1'000'000, 3);
}

TOOLBOX_BENCHMARK(histogram_reset_1e7_3sf)
{
    // Record microseconds with 3sf and max expected value of ten seconds.
    histogram_reset(ctx, 1, 10'000'000, 3);
}

TOOLBOX_BENCHMARK(histogram_reset_1e6_5sf)
{
    // Record microseconds with 5sf and max expected value of one second.
    histogram_reset(ctx, 1, 1'000'000, 5);
}

TOOLBOX_BENCHMARK(histogram_reset_1e7_5sf)
{
    // Record microseconds with 5sf and max expected value of ten seconds.
    histogram_reset(ctx, 1, 10'000'000, 5);
}

TOOLBOX_BENCHMARK(histogram_record_value)
{
    // Record microseconds with 3sf and max expected value of one second.
    std::unique_ptr<Histogram> hist{new Histogram{1, 1'000'000, 3}};
    std::uint64_t val{0};
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(100)) {
            hist->record_value(1 + val % 1'000'000);
        }
    }
}

} // namespace
