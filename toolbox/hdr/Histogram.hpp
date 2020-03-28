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
#ifndef TOOLBOX_HDR_HISTOGRAM
#define TOOLBOX_HDR_HISTOGRAM

#include <toolbox/Config.h>

#include <cstdint>
#include <vector>

namespace toolbox {
/// A C++ port of HdrHistogram_c written Michael Barker and released to the public domain.
inline namespace hdr {
/// Bucket configuration.
struct TOOLBOX_API HdrBucketConfig {
    /// Construct HdrBucketConfig.
    ///
    /// Due to the size of the histogram being the result of some reasonably involved math on the
    /// input parameters.
    ///
    /// \param lowest_trackable_value The smallest possible value to be put into the histogram.
    /// \param highest_trackable_value The largest possible value to be put into the histogram.
    /// \param significant_figures The level of precision for this histogram, i.e. the number of
    /// figures in a decimal number that will be maintained. E.g. a value of 3 will mean the results
    /// from the histogram will be accurate up to the first three digits. Must be a value between 1
    /// and 5 (inclusive).
    HdrBucketConfig(std::int64_t lowest_trackable_value, std::int64_t highest_trackable_value,
                    int significant_figures);

    // Copy.
    HdrBucketConfig(const HdrBucketConfig&) noexcept = default;
    HdrBucketConfig& operator=(const HdrBucketConfig&) noexcept = default;

    // Move.
    HdrBucketConfig(HdrBucketConfig&&) noexcept = default;
    HdrBucketConfig& operator=(HdrBucketConfig&&) noexcept = default;

    std::int64_t lowest_trackable_value;
    std::int64_t highest_trackable_value;
    std::int32_t significant_figures;
    std::int32_t unit_magnitude;
    std::int32_t sub_bucket_half_count_magnitude;
    std::int32_t sub_bucket_count;
    std::int32_t sub_bucket_half_count;
    std::int64_t sub_bucket_mask;
    std::int32_t bucket_count;
    std::int32_t counts_len;
};

/// A High Dynamic Range (HDR) Histogram.
class TOOLBOX_API HdrHistogram {
  public:
    HdrHistogram(const HdrBucketConfig& config);
    HdrHistogram(std::int64_t lowest_trackable_value, std::int64_t highest_trackable_value,
                 std::int32_t significant_figures);

    // Copy.
    HdrHistogram(const HdrHistogram&) = default;
    HdrHistogram& operator=(const HdrHistogram&) = default;

    // Move.
    HdrHistogram(HdrHistogram&&) noexcept = default;
    HdrHistogram& operator=(HdrHistogram&&) noexcept = default;

    std::int64_t lowest_trackable_value() const noexcept { return lowest_trackable_value_; }
    std::int64_t highest_trackable_value() const noexcept { return highest_trackable_value_; }
    std::int32_t significant_figures() const noexcept { return significant_figures_; }

    std::int32_t sub_bucket_count() const noexcept { return sub_bucket_count_; }
    std::int32_t bucket_count() const noexcept { return bucket_count_; }
    std::int64_t total_count() const noexcept { return total_count_; }
    std::int32_t counts_len() const noexcept { return static_cast<std::int32_t>(counts_.size()); }

    /// Get minimum value from the histogram. Will return 2^63-1 if the histogram is empty.
    std::int64_t min() const noexcept;

    /// Get maximum value from the histogram. Will return 0 if the histogram is empty.
    std::int64_t max() const noexcept;

    /// Determine if two values are equivalent with the histogram's resolution. Where "equivalent"
    /// means that value samples recorded for any two equivalent values are counted in a common
    /// total count.
    ///
    /// \param a first value to compare.
    /// \param b second value to compare.
    /// \return true if values are equivalent with the histogram's resolution.
    bool values_are_equivalent(std::int64_t a, std::int64_t b) const noexcept;

    /// Get the lowest value that is equivalent to the given value within the histogram's
    /// resolution. Where "equivalent" means that value samples recorded for any two equivalent
    /// values are counted in a common total count.
    ///
    /// \param value The given value.
    /// \return the lowest value that is equivalent to the given value within the histogram's
    /// resolution.
    std::int64_t lowest_equivalent_value(std::int64_t value) const noexcept;
    std::int64_t highest_equivalent_value(std::int64_t value) const noexcept;

    /// Get the count of recorded values at a specific value (to within the histogram resolution at
    /// the value level).
    ///
    /// \param value The value for which to provide the recorded count.
    /// \return The total count of values recorded in the histogram within the value range.
    std::int64_t count_at_value(std::int64_t value) const noexcept;
    std::int64_t count_at_index(std::int32_t index) const noexcept;
    std::int64_t value_at_index(std::int32_t index) const noexcept;
    std::int64_t size_of_equivalent_value_range(std::int64_t value) const noexcept;
    std::int64_t next_non_equivalent_value(std::int64_t value) const noexcept;
    std::int64_t median_equivalent_value(std::int64_t value) const noexcept;
    std::int64_t counts_get_normalised(std::int32_t index) const noexcept;

    /// Reset a histogram to zero - empty out a histogram and re-initialise it.
    ///
    /// If you want to re-use an existing histogram, but reset everything back to zero, this is the
    /// routine to use.
    void reset() noexcept;

    /// Records a value in the histogram, will round this value of to a precision at or better than
    /// the significant_figure specified at construction time.
    ///
    /// \param value Value to add to the histogram.
    /// \return false if the value is larger than the highest_trackable_value and can't be recorded,
    /// true otherwise.
    bool record_value(std::int64_t value) noexcept;

    /// Records count values in the histogram, will round this value of to a precision at or better
    /// than the significant_figure specified at construction time.
    ///
    /// \param value Value to add to the histogram.
    /// \param count Number of values to add to the histogram.
    /// \return false if any value is larger than the highest_trackable_value and can't be recorded,
    /// true otherwise.
    bool record_values(std::int64_t value, std::int64_t count) noexcept;

  private:
    std::int32_t normalize_index(std::int32_t index) const noexcept;
    std::int32_t get_bucket_index(std::int64_t value) const noexcept;
    std::int32_t counts_index(std::int32_t bucket_index, std::int32_t sub_bucket_index) const
        noexcept;
    std::int32_t counts_index_for(std::int64_t value) const noexcept;
    std::int64_t non_zero_min() const noexcept;

    void counts_inc_normalised(std::int32_t index, std::int64_t value) noexcept;
    void update_min_max(std::int64_t value) noexcept;

    std::int64_t lowest_trackable_value_;
    std::int64_t highest_trackable_value_;
    std::int32_t significant_figures_;
    std::int32_t unit_magnitude_;
    std::int32_t sub_bucket_half_count_magnitude_;
    std::int32_t sub_bucket_count_;
    std::int32_t sub_bucket_half_count_;
    std::int64_t sub_bucket_mask_;
    std::int32_t bucket_count_;

    std::int32_t normalizing_index_offset_;
    std::int64_t min_value_;
    std::int64_t max_value_;
    std::int64_t total_count_;
    std::vector<std::int64_t> counts_;
};

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_HISTOGRAM
