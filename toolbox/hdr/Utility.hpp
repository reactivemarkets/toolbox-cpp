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
#ifndef TOOLBOX_HDR_UTILITY
#define TOOLBOX_HDR_UTILITY

#include <toolbox/Config.h>

#include <cstdint>
#include <iosfwd>

namespace toolbox {
/// A C++ port of HdrHistogram_c written Michael Barker and released to the public domain.
inline namespace hdr {
class HdrHistogram;

TOOLBOX_API std::int64_t min(const HdrHistogram& h) noexcept;
TOOLBOX_API std::int64_t max(const HdrHistogram& h) noexcept;

/// Get the value at a specific percentile.
///
/// \param h The histogram.
/// \param percentile The percentile to get the value for.
/// \return the percentile value.
TOOLBOX_API std::int64_t value_at_percentile(const HdrHistogram& h, double percentile) noexcept;

/// Gets the mean for the values in the histogram.
///
/// \param h The histogram.
/// \return the mean.
TOOLBOX_API double mean(const HdrHistogram& h) noexcept;

/// Gets the standard deviation for the values in the histogram.
///
/// \param h The histogram.
/// \return the standard deviation.
TOOLBOX_API double stddev(const HdrHistogram& h) noexcept;

struct PutPercentiles {
    const HdrHistogram& h;
    std::int32_t ticks_per_half_distance{5};
    double value_scale{1000.0};
};

inline auto put_percentiles(const HdrHistogram& h, std::int32_t ticks_per_half_distance,
                            double value_scale) noexcept
{
    return PutPercentiles{h, ticks_per_half_distance, value_scale};
}

TOOLBOX_API std::ostream& operator<<(std::ostream& os, PutPercentiles pp);

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_UTILITY
