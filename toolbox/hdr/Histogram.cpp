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

#include "Histogram.hpp"

#include <cmath>
#include <stdexcept>

namespace toolbox {
inline namespace hdr {
using namespace std;
namespace {

// Smallest power of 2 containing value.
int32_t count_leading_zeros_64(int64_t value) noexcept
{
    return __builtin_clzll(value);
}

int32_t get_sub_bucket_index(int64_t value, int32_t bucket_index, int32_t unit_magnitude) noexcept
{
    return value >> (bucket_index + unit_magnitude);
}

int32_t buckets_needed_to_cover_value(int64_t value, int64_t sub_bucket_count,
                                      int32_t unit_magnitude) noexcept
{
    int64_t smallest_untrackable_value{sub_bucket_count << unit_magnitude};
    int32_t buckets_needed{1};
    while (smallest_untrackable_value <= value) {
        if (smallest_untrackable_value > numeric_limits<int64_t>::max() / 2) {
            return buckets_needed + 1;
        }
        smallest_untrackable_value <<= 1;
        ++buckets_needed;
    }
    return buckets_needed;
}

int64_t value_from_index(int32_t bucket_index, int32_t sub_bucket_index,
                         int32_t unit_magnitude) noexcept
{
    return int64_t{sub_bucket_index} << (bucket_index + unit_magnitude);
}

int32_t get_sub_bucket_half_count_magnitude(int64_t significant_figures) noexcept
{
    const double largest_value_with_single_unit_resolution{pow(10, significant_figures) * 2.0};
    const int32_t sub_bucket_count_magnitude
        = ceil(log(largest_value_with_single_unit_resolution) / log(2));
    return sub_bucket_count_magnitude > 1 ? sub_bucket_count_magnitude - 1 : 0;
}

} // namespace

HdrBucketConfig::HdrBucketConfig(int64_t lowest_trackable_value, int64_t highest_trackable_value,
                                 int32_t significant_figures)
: lowest_trackable_value{lowest_trackable_value}
, highest_trackable_value{highest_trackable_value}
, significant_figures{significant_figures}
, unit_magnitude(floor(log(lowest_trackable_value) / log(2)))
, sub_bucket_half_count_magnitude{get_sub_bucket_half_count_magnitude(significant_figures)}
, sub_bucket_count(pow(2, sub_bucket_half_count_magnitude + 1))
, sub_bucket_half_count{sub_bucket_count / 2}
, sub_bucket_mask{int64_t{sub_bucket_count - 1} << unit_magnitude}
, bucket_count{buckets_needed_to_cover_value(highest_trackable_value, sub_bucket_count,
                                             unit_magnitude)}
, counts_len{(bucket_count + 1) * (sub_bucket_count / 2)}
{
    if (lowest_trackable_value < 1) {
        throw invalid_argument{"min value must be greater than zero"};
    }
    if (significant_figures < 1 || significant_figures > 5) {
        throw invalid_argument{"significant figures must be between 1 and 5"};
    }
    if (lowest_trackable_value * 2 > highest_trackable_value) {
        throw invalid_argument{"highest trackable value too small"};
    }
    if (unit_magnitude + sub_bucket_half_count_magnitude > 61) {
        throw invalid_argument{"invalid magnitude"};
    }
}

HdrHistogram::HdrHistogram(const HdrBucketConfig& config)
: lowest_trackable_value_{config.lowest_trackable_value}
, highest_trackable_value_{config.highest_trackable_value}
, significant_figures_{config.significant_figures}
, unit_magnitude_{config.unit_magnitude}
, sub_bucket_half_count_magnitude_{config.sub_bucket_half_count_magnitude}
, sub_bucket_count_{config.sub_bucket_count}
, sub_bucket_half_count_{config.sub_bucket_half_count}
, sub_bucket_mask_{config.sub_bucket_mask}
, bucket_count_{config.bucket_count}
, normalizing_index_offset_{0}
, min_value_{numeric_limits<int64_t>::max()}
, max_value_{0}
, total_count_{0}
{
    counts_.resize(config.counts_len);
}

HdrHistogram::HdrHistogram(int64_t lowest_trackable_value, int64_t highest_trackable_value,
                           int significant_figures)
: HdrHistogram{
    HdrBucketConfig{lowest_trackable_value, highest_trackable_value, significant_figures}}
{
}

int64_t HdrHistogram::min() const noexcept
{
    if (count_at_index(0) > 0) {
        return 0;
    }
    return non_zero_min();
}

int64_t HdrHistogram::max() const noexcept
{
    if (max_value_ == 0) {
        return 0;
    }
    return highest_equivalent_value(max_value_);
}

bool HdrHistogram::values_are_equivalent(int64_t a, int64_t b) const noexcept
{
    return lowest_equivalent_value(a) == lowest_equivalent_value(b);
}

int64_t HdrHistogram::lowest_equivalent_value(int64_t value) const noexcept
{
    const int32_t bucket_index{get_bucket_index(value)};
    const int32_t sub_bucket_index{get_sub_bucket_index(value, bucket_index, unit_magnitude_)};
    return value_from_index(bucket_index, sub_bucket_index, unit_magnitude_);
}

int64_t HdrHistogram::highest_equivalent_value(int64_t value) const noexcept
{
    return next_non_equivalent_value(value) - 1;
}

int64_t HdrHistogram::count_at_value(int64_t value) const noexcept
{
    return counts_get_normalised(counts_index_for(value));
}

int64_t HdrHistogram::count_at_index(int32_t index) const noexcept
{
    return counts_get_normalised(index);
}

int64_t HdrHistogram::value_at_index(int32_t index) const noexcept
{
    int32_t bucket_index{(index >> sub_bucket_half_count_magnitude_) - 1};
    int32_t sub_bucket_index{(index & (sub_bucket_half_count_ - 1)) + sub_bucket_half_count_};

    if (bucket_index < 0) {
        sub_bucket_index -= sub_bucket_half_count_;
        bucket_index = 0;
    }
    return value_from_index(bucket_index, sub_bucket_index, unit_magnitude_);
}

int64_t HdrHistogram::size_of_equivalent_value_range(int64_t value) const noexcept
{
    const int32_t bucket_index{get_bucket_index(value)};
    const int32_t sub_bucket_index{get_sub_bucket_index(value, bucket_index, unit_magnitude_)};
    const int32_t adjusted_bucket{(sub_bucket_index >= sub_bucket_count_) ? (bucket_index + 1)
                                                                          : bucket_index};
    return int64_t{1} << (unit_magnitude_ + adjusted_bucket);
}

int64_t HdrHistogram::next_non_equivalent_value(int64_t value) const noexcept
{
    return lowest_equivalent_value(value) + size_of_equivalent_value_range(value);
}

int64_t HdrHistogram::median_equivalent_value(int64_t value) const noexcept
{
    return lowest_equivalent_value(value) + (size_of_equivalent_value_range(value) >> 1);
}

int64_t HdrHistogram::counts_get_normalised(int32_t index) const noexcept
{
    return counts_[normalize_index(index)];
}

void HdrHistogram::reset() noexcept
{
    min_value_ = numeric_limits<int64_t>::max();
    max_value_ = 0;
    total_count_ = 0;
    fill(counts_.begin(), counts_.end(), 0);
}

bool HdrHistogram::record_value(int64_t value) noexcept
{
    return record_values(value, 1);
}

bool HdrHistogram::record_values(int64_t value, int64_t count) noexcept
{
    if (value < 0) {
        return false;
    }
    const int32_t counts_index{counts_index_for(value)};
    if (counts_index < 0 || counts_len() <= counts_index) {
        return false;
    }
    counts_inc_normalised(counts_index, count);
    update_min_max(value);
    return true;
}

int32_t HdrHistogram::normalize_index(int32_t index) const noexcept
{
    if (normalizing_index_offset_ == 0) {
        return index;
    }

    int32_t normalized_index{index - normalizing_index_offset_};

    int32_t adjustment{0};
    if (normalized_index < 0) {
        adjustment = counts_len();
    } else if (normalized_index >= counts_len()) {
        adjustment = -counts_len();
    }
    return normalized_index + adjustment;
}

int32_t HdrHistogram::get_bucket_index(int64_t value) const noexcept
{
    // Smallest power of 2 containing value.
    const int32_t pow2ceiling{64 - count_leading_zeros_64(value | sub_bucket_mask_)};
    return pow2ceiling - unit_magnitude_ - (sub_bucket_half_count_magnitude_ + 1);
}

int32_t HdrHistogram::counts_index(int32_t bucket_index, int32_t sub_bucket_index) const noexcept
{
    // Calculate the index for the first entry in the bucket.
    // The following is the equivalent of ((bucket_index + 1) * subBucketHalfCount)).
    const int32_t bucket_base_index{(bucket_index + 1) << sub_bucket_half_count_magnitude_};
    // Calculate the offset in the bucket.
    const int32_t offset_in_bucket{sub_bucket_index - sub_bucket_half_count_};
    // The following is the equivalent of
    //  (sub_bucket_index  - subBucketHalfCount) + bucketBaseIndex).
    return bucket_base_index + offset_in_bucket;
}

int32_t HdrHistogram::counts_index_for(int64_t value) const noexcept
{
    const int32_t bucket_index{get_bucket_index(value)};
    const int32_t sub_bucket_index{get_sub_bucket_index(value, bucket_index, unit_magnitude_)};
    return counts_index(bucket_index, sub_bucket_index);
}

int64_t HdrHistogram::non_zero_min() const noexcept
{
    if (min_value_ == numeric_limits<int64_t>::max()) {
        return min_value_;
    }
    return lowest_equivalent_value(min_value_);
}

void HdrHistogram::counts_inc_normalised(int32_t index, int64_t value) noexcept
{
    const int32_t normalised_index{normalize_index(index)};
    counts_[normalised_index] += value;
    total_count_ += value;
}

void HdrHistogram::update_min_max(int64_t value) noexcept
{
    if (value != 0) {
        min_value_ = std::min(min_value_, value);
    }
    max_value_ = std::max(max_value_, value);
}

} // namespace hdr
} // namespace toolbox
