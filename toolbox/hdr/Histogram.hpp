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
#ifndef TOOLBOX_HDR_HISTOGRAM
#define TOOLBOX_HDR_HISTOGRAM

#include "Iterator.hpp"

#include <toolbox/Config.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace toolbox {
inline namespace hdr {

class TOOLBOX_API HdrHistogram {
    friend class HdrIterator;
    friend class RecordedIterator;
    friend class PercentileIterator;
    friend class AllValuesIterator;
    friend std::ostream& operator<<(std::ostream& out, const HdrHistogram& hist);

  public:
    HdrHistogram(std::int64_t lowest_value, std::int64_t highest_value,
                 std::int64_t significant_figures);

    // Copy.
    HdrHistogram(const HdrHistogram&) = default;
    HdrHistogram& operator=(const HdrHistogram&) = default;

    // Move.
    HdrHistogram(HdrHistogram&&) noexcept = default;
    HdrHistogram& operator=(HdrHistogram&&) noexcept = default;

    PercentileIterator begin() const;
    PercentileIterator end() const;

    std::int64_t percentile(double percentile) const;

    /// Check whether two values are equivalent (meaning they
    /// are in the same bucket/range)
    /// Returns:
    ///    true if the two values are equivalentv
    std::int64_t values_are_equivalent(std::int64_t lhs, std::int64_t rhs) const noexcept;

    std::int64_t max() const noexcept;
    std::int64_t min() const noexcept;
    double mean() const noexcept;
    double stddev() const noexcept;

    std::int64_t get_count_at_value(std::int64_t value) const noexcept;
    std::int64_t get_total_count() const noexcept;

    std::int64_t get_highest_equivalent_value(std::int64_t value) const noexcept;
    std::int64_t get_lowest_equivalent_value(std::int64_t value) const noexcept;

    bool record(std::int64_t value, int count = 1) noexcept;
    void reset() noexcept;

    std::int64_t unit_magnitude;
    std::int64_t sub_bucket_half_count_magnitude;
    std::int64_t sub_bucket_count;
    std::int64_t sub_bucket_half_count;
    std::int64_t sub_bucket_mask;
    std::int64_t bucket_count;
    std::int64_t counts_len;

  private:
    std::int64_t get_value_from_sub_bucket(std::int64_t bucket_index,
                                           std::int64_t sub_bucket_index) const noexcept;
    std::int64_t get_count_at_index(std::int64_t index) const;
    std::int64_t get_target_count_at_percentile(double percentile) const noexcept;
    std::int64_t counts_index_for(std::int64_t value) const noexcept;

    std::int64_t get_bucket_index(std::int64_t value) const noexcept;
    std::int64_t get_sub_bucket_index(std::int64_t value, std::int64_t bucket_index) const noexcept;

    std::int64_t get_value_from_index(std::int64_t index) const noexcept;
    std::int64_t counts_index(int bucket_index, int sub_bucket_index) const noexcept;

    std::int64_t hdr_size_of_equiv_value_range(std::int64_t value) const noexcept;
    std::int64_t hdr_median_equiv_value(std::int64_t value) const noexcept;

    std::int64_t min_value_{std::numeric_limits<std::int64_t>::max()};
    std::int64_t max_value_{0};

    std::int64_t total_count_{0};
    std::vector<std::int64_t> counts_;

    // FIXME: what is the purpose of this data member?
    static constexpr double int_to_double_conversion_ratio_{1.0};
};

TOOLBOX_API std::ostream& operator<<(std::ostream& out, const HdrHistogram& hist);

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_HISTOGRAM
