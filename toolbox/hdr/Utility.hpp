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
#ifndef TOOLBOX_HDR_UTILITY
#define TOOLBOX_HDR_UTILITY

#include "Iterator.hpp"
#include "Histogram.hpp"

#include <toolbox/util/Concepts.hpp>
#include <toolbox/Config.h>

#include <cstdint>
#include <format>

namespace toolbox {
/// A C++ port of HdrHistogram_c written Michael Barker and released to the public domain.
inline namespace hdr {
class Histogram;

TOOLBOX_API std::int64_t min(const Histogram& h) noexcept;
TOOLBOX_API std::int64_t max(const Histogram& h) noexcept;

/// Get the value at a specific percentile.
///
/// \param h The histogram.
/// \param percentile The percentile to get the value for.
/// \return the percentile value.
TOOLBOX_API std::int64_t value_at_percentile(const Histogram& h, double percentile) noexcept;

/// Gets the mean for the values in the histogram.
///
/// \param h The histogram.
/// \return the mean.
TOOLBOX_API double mean(const Histogram& h) noexcept;

/// Gets the standard deviation for the values in the histogram.
///
/// \param h The histogram.
/// \return the standard deviation.
TOOLBOX_API double stddev(const Histogram& h) noexcept;

struct PutPercentiles {
    const Histogram& h;
    std::int32_t ticks_per_half_distance{5};
    double value_scale{1000.0};
};

inline auto put_percentiles(const Histogram& h, std::int32_t ticks_per_half_distance,
                            double value_scale) noexcept
{
    return PutPercentiles{h, ticks_per_half_distance, value_scale};
}

template <typename StreamT>
    requires Streamable<StreamT>
StreamT& operator<<(StreamT& os, PutPercentiles pp)
{
    const auto sf = pp.h.significant_figures();

    os << "       Value     Percentile TotalCount 1/(1-Percentile)\n\n";

    PercentileIterator iter{pp.h, pp.ticks_per_half_distance};
    while (iter.next()) {
        const double value{iter.highest_equivalent_value() / pp.value_scale};
        const double percentile{iter.percentile() / 100.0};
        const int64_t total_count{iter.cumulative_count()};

        os << std::format("{:12.{}f}", value, sf);
        os << std::format("{:15.6f}", percentile);
        os << std::format("{:11}", total_count);

        if (percentile < 1.0) {
            const double inverted_percentile{(1.0 / (1.0 - percentile))};
            os << std::format("{:15.2f}", inverted_percentile);
        }
        os << '\n';
    }

    const double mean_val{mean(pp.h) / pp.value_scale};
    const double stddev_val{stddev(pp.h)};
    const double max_val{pp.h.max() / pp.value_scale};
    const int64_t total_val{pp.h.total_count()};

    // clang-format off
    os << "#[Mean    = " << std::format("{:12.{}f}", mean_val, sf)
       << ", StdDeviation   = " << std::format("{:12.{}f}", stddev_val, sf)
       << "]\n#[Max     = " << std::format("{:12.{}f}", max_val, sf)
       << ", TotalCount     = " << std::format("{:12}", total_val)
       << "]\n#[Buckets = " << std::format("{:12}", pp.h.bucket_count())
       << ", SubBuckets     = " << std::format("{:12}", pp.h.sub_bucket_count())
       << "]";
    return os;
}


} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_UTILITY
