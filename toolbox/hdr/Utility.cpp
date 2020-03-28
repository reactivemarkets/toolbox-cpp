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

#include "Utility.hpp"

#include "Histogram.hpp"
#include "Iterator.hpp"

#include <boost/io/ios_state.hpp>

#include <cmath>
#include <iomanip>
#include <ostream>

namespace toolbox {
inline namespace hdr {
using namespace std;
namespace {
int64_t get_count_at_percentile(const HdrHistogram& h, double percentile) noexcept
{
    if (percentile > 100.0) {
        percentile = 100.0;
    }
    const int count_at_percentile = (percentile * h.total_count() / 100) + 0.5;
    return std::max<int64_t>(count_at_percentile, 1);
}
} // namespace

int64_t min(const HdrHistogram& h) noexcept
{
    return h.min();
}

int64_t max(const HdrHistogram& h) noexcept
{
    return h.max();
}

int64_t value_at_percentile(const HdrHistogram& h, double percentile) noexcept
{
    const int64_t count_at_percentile{get_count_at_percentile(h, percentile)};

    int64_t total{0};
    HdrIterator iter{h};
    while (iter.next()) {
        total += iter.count();
        if (total >= count_at_percentile) {
            return h.highest_equivalent_value(iter.value());
        }
    }
    return 0;
}

double mean(const HdrHistogram& h) noexcept
{
    const auto total_count = h.total_count();

    int64_t total{0};
    HdrIterator iter{h};
    while (iter.next()) {
        if (iter.count() != 0) {
            total += iter.count() * h.median_equivalent_value(iter.value());
        }
    }
    return double(total) / total_count;
}

double stddev(const HdrHistogram& h) noexcept
{
    const int64_t total_count{h.total_count()};
    const double mean_val{mean(h)};

    double geometric_dev_total{0.0};
    HdrIterator iter{h};
    while (iter.next()) {
        if (iter.count() != 0) {
            const double dev{h.median_equivalent_value(iter.value()) - mean_val};
            geometric_dev_total += (dev * dev) * iter.count();
        }
    }
    return sqrt(geometric_dev_total / total_count);
}

ostream& operator<<(ostream& os, PutPercentiles pp)
{
    const auto sf = pp.h.significant_figures();
    boost::io::ios_all_saver all_saver{os};

    os << "       Value     Percentile TotalCount 1/(1-Percentile)\n\n";

    HdrPercentileIterator iter{pp.h, pp.ticks_per_half_distance};
    while (iter.next()) {
        const double value{iter.highest_equivalent_value() / pp.value_scale};
        const double percentile{iter.percentile() / 100.0};
        const int64_t total_count{iter.cumulative_count()};

        // clang-format off
        os << setw(12) << fixed << setprecision(sf) << value
           << setw(15) << fixed << setprecision(6) << percentile
           << setw(11) << total_count;
        // clang-format on

        if (percentile < 1.0) {
            const double inverted_percentile{(1.0 / (1.0 - percentile))};
            os << setw(15) << fixed << setprecision(2) << inverted_percentile;
        }
        os << '\n';
    }

    const double mean_val{mean(pp.h) / pp.value_scale};
    const double stddev_val{stddev(pp.h)};
    const double max_val{pp.h.max() / pp.value_scale};
    const int64_t total_val{pp.h.total_count()};

    // clang-format off
    return os
        << "#[Mean    = " << setw(12) << fixed << setprecision(sf) << mean_val
        << ", StdDeviation   = " << setw(12) << fixed << setprecision(sf) << stddev_val
        << "]\n"
        "#[Max     = " << setw(12) << fixed << setprecision(sf) << max_val
        << ", TotalCount     = " << setw(12) << total_val
        << "]\n"
        "#[Buckets = " << setw(12) << pp.h.bucket_count()
        << ", SubBuckets     = " << setw(12) << pp.h.sub_bucket_count()
        << "]";
    // clang-format on
}

} // namespace hdr
} // namespace toolbox
