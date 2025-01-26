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

#include "Utility.hpp"

#include "Histogram.hpp"
#include "Iterator.hpp"

#include <cmath>

namespace toolbox {
inline namespace hdr {
using namespace std;
namespace {
int64_t get_count_at_percentile(const Histogram& h, double percentile) noexcept
{
    if (percentile > 100.0) {
        percentile = 100.0;
    }
    const int count_at_percentile = (percentile * h.total_count() / 100) + 0.5;
    return std::max<int64_t>(count_at_percentile, 1);
}
} // namespace

int64_t min(const Histogram& h) noexcept
{
    return h.min();
}

int64_t max(const Histogram& h) noexcept
{
    return h.max();
}

int64_t value_at_percentile(const Histogram& h, double percentile) noexcept
{
    const int64_t count_at_percentile{get_count_at_percentile(h, percentile)};

    int64_t total{0};
    Iterator iter{h};
    while (iter.next()) {
        total += iter.count();
        if (total >= count_at_percentile) {
            return h.highest_equivalent_value(iter.value());
        }
    }
    return 0;
}

double mean(const Histogram& h) noexcept
{
    const auto total_count = h.total_count();

    int64_t total{0};
    Iterator iter{h};
    while (iter.next()) {
        if (iter.count() != 0) {
            total += iter.count() * h.median_equivalent_value(iter.value());
        }
    }
    return double(total) / total_count;
}

double stddev(const Histogram& h) noexcept
{
    const int64_t total_count{h.total_count()};
    const double mean_val{mean(h)};

    double geometric_dev_total{0.0};
    Iterator iter{h};
    while (iter.next()) {
        if (iter.count() != 0) {
            const double dev{h.median_equivalent_value(iter.value()) - mean_val};
            geometric_dev_total += (dev * dev) * iter.count();
        }
    }
    return sqrt(geometric_dev_total / total_count);
}

} // namespace hdr
} // namespace toolbox
