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

#include "HdrHistogram.hpp"

namespace {
std::int64_t get_bucket_count(std::int64_t value, std::int64_t subb_count,
                              std::int64_t unit_mag) noexcept
{
    auto smallest_untrackable_value = subb_count << unit_mag;
    auto buckets_needed = 1;
    while (smallest_untrackable_value <= value) {
        if (smallest_untrackable_value > (std::numeric_limits<std::int64_t>::max() / 2)) {
            return buckets_needed + 1;
        }
        smallest_untrackable_value <<= 1;
        ++buckets_needed;
    }
    return buckets_needed;
}

std::int64_t init_sub_bucket_half_count_magnitude(std::int64_t significant_figures)
{
    double largest_value_single_unit_res = 2 * std::pow(10, significant_figures);
    auto subb_count_mag = static_cast<std::int64_t>(
        std::ceil(std::log(largest_value_single_unit_res) / std::log(2)));
    return subb_count_mag > 1 ? subb_count_mag - 1 : 0;
}

} // namespace

namespace toolbox {

inline namespace hdr {

HdrHistogram::HdrHistogram(std::int64_t lowest_value, std::int64_t highest_value,
                           std::int64_t significant_figures)
: unit_magnitude{static_cast<std::int64_t>(std::floor(std::log(lowest_value) / std::log(2)))}
, sub_bucket_half_count_magnitude{init_sub_bucket_half_count_magnitude(significant_figures)}
, sub_bucket_count(std::pow(2, sub_bucket_half_count_magnitude + 1))
, sub_bucket_half_count{sub_bucket_count / 2}
, sub_bucket_mask{(sub_bucket_count - 1) << unit_magnitude}
, bucket_count{get_bucket_count(highest_value, sub_bucket_count, unit_magnitude)}
, counts_len{(bucket_count + 1) * (sub_bucket_count / 2)}
{
    if (highest_value < 1) {
        throw std::runtime_error{"Min value must be 1 or greater"};
    }
    if (significant_figures < 1 || significant_figures > 5) {
        throw std::runtime_error{"SignificantFigures value must be between 1 and 5"};
    }
    counts_.resize(counts_len);
}

bool HdrHistogram::record(std::int64_t value, int count)
{
    auto counts_index = counts_index_for(value);
    if (counts_len <= counts_index) {
        return false;
    }
    counts_[counts_index] += count;
    total_count_ += count;
    min_value_ = std::min(min_value_, value);
    max_value_ = std::max(max_value_, value);
    return true;
}

std::int64_t HdrHistogram::percentile(double percentile) const noexcept
{
    auto count_at_percentile = get_target_count_at_percentile(percentile);
    std::int64_t total = 0;
    for (std::int64_t i = 0; i < counts_len; ++i) {
        total += get_count_at_index(i);
        if (total >= count_at_percentile) {
            auto value_at_index = get_value_from_index(i);
            return percentile ? get_highest_equivalent_value(value_at_index)
                              : get_lowest_equivalent_value(value_at_index);
        }
    }
    return 0;
}

std::int64_t HdrHistogram::values_are_equivalent(std::int64_t lhs, std::int64_t rhs)
{
    return get_lowest_equivalent_value(lhs) == get_lowest_equivalent_value(rhs);
}

std::int64_t HdrHistogram::max() const noexcept
{
    return max_value_ ? get_highest_equivalent_value(max_value_) : 0;
}

std::int64_t HdrHistogram::min() const noexcept
{
    if (total_count_ == 0 || counts_[0] > 0) {
        return 0;
    }
    if (std::numeric_limits<std::int64_t>::max() == min_value_) {
        return min_value_;
    }
    return get_lowest_equivalent_value(min_value_);
}

using RecordedIteratorAdapter = IteratorAdapter<RecordedIterator>;

double HdrHistogram::mean() const noexcept
{
    if (!total_count_) {
        return 0.0;
    }
    double total{0.0};
    auto iter_adapter = RecordedIteratorAdapter(*this);
    const auto& end = iter_adapter.end();
    for (auto itr = iter_adapter.begin(); itr != end; ++itr) {
        total += itr.get_count_at_this_value() * hdr_median_equiv_value(itr->value_iterated_to);
    }
    return total / total_count_;
}

double HdrHistogram::stddev() const noexcept
{
    if (!total_count_) {
        return 0.0;
    }
    const auto mean_value = mean();
    double geometric_dev_total{0.0};
    for (const auto& item : RecordedIteratorAdapter(*this)) {
        const auto dev = (hdr_median_equiv_value(item.value_iterated_to) * 1.0) - mean_value;
        geometric_dev_total += (dev * dev) * item.count_added_in_this_iter_step;
    }
    return std::sqrt(geometric_dev_total / total_count_);
}

std::int64_t HdrHistogram::get_count_at_value(std::int64_t value) const noexcept
{
    auto counts_index = counts_index_for(value);
    return counts_[counts_index];
}

std::int64_t HdrHistogram::get_total_count() const noexcept
{
    return total_count_;
}

std::int64_t HdrHistogram::get_highest_equivalent_value(std::int64_t value) const noexcept
{
    auto bucket_index = get_bucket_index(value);
    auto sub_bucket_index = get_sub_bucket_index(value, bucket_index);

    auto lowest_equivalent_value = get_value_from_sub_bucket(bucket_index, sub_bucket_index);
    if (sub_bucket_index >= sub_bucket_count) {
        ++bucket_index;
    }
    auto size_of_equivalent_value_range = 1ul << (unit_magnitude + bucket_index);
    auto next_non_equivalent_value = lowest_equivalent_value + size_of_equivalent_value_range;
    return next_non_equivalent_value - 1;
}

std::int64_t HdrHistogram::get_lowest_equivalent_value(std::int64_t value) const noexcept
{
    auto bucket_index = get_bucket_index(value);
    auto sub_bucket_index = get_sub_bucket_index(value, bucket_index);
    auto lowest_equivalent_value = get_value_from_sub_bucket(bucket_index, sub_bucket_index);
    return lowest_equivalent_value;
}

std::int64_t HdrHistogram::get_value_from_sub_bucket(std::int64_t bucket_index,
                                                     std::int64_t sub_bucket_index) const noexcept
{
    return sub_bucket_index << (bucket_index + unit_magnitude);
}

std::int64_t HdrHistogram::get_count_at_index(std::int64_t index) const noexcept
{
    // some decoded (read-only) histograms may have truncated
    // counts arrays, we return zero for any index that is passed the array
    //         if (index >= encoder.payload.counts_len){
    //             return 0;
    //         }
    return counts_.at(index);
}

std::int64_t HdrHistogram::get_target_count_at_percentile(double percentile) const noexcept
{
    const auto requested_percentile = std::min(percentile, 100.0);
    const int count_at_percentile = (requested_percentile * total_count_ / 100) + 0.5;
    return std::max(count_at_percentile, 1);
}

std::int64_t HdrHistogram::counts_index_for(std::int64_t value) const noexcept
{
    auto bucket_index = get_bucket_index(value);
    auto sub_bucket_index = get_sub_bucket_index(value, bucket_index);
    return counts_index(bucket_index, sub_bucket_index);
}

std::int64_t HdrHistogram::get_bucket_index(std::int64_t value) const noexcept
{
    // Smallest power of 2 containing value
    auto pow2ceiling = 64 - __builtin_clzll(value | sub_bucket_mask);
    return pow2ceiling - unit_magnitude - (sub_bucket_half_count_magnitude + 1);
}

std::int64_t HdrHistogram::get_sub_bucket_index(std::int64_t value, std::int64_t bucket_index) const
    noexcept
{
    return value >> (bucket_index + unit_magnitude);
}

std::int64_t HdrHistogram::get_value_from_index(std::int64_t index) const noexcept
{
    int bucket_index = (index >> sub_bucket_half_count_magnitude) - 1;
    auto sub_bucket_index = (index & (sub_bucket_half_count - 1)) + sub_bucket_half_count;
    if (bucket_index < 0) {
        sub_bucket_index -= sub_bucket_half_count;
        bucket_index = 0;
    }
    return get_value_from_sub_bucket(bucket_index, sub_bucket_index);
}

std::int64_t HdrHistogram::counts_index(int bucket_index, int sub_bucket_index) const noexcept
{
    // Calculate the index for the first entry in the bucket:
    // (The following is the equivalent of ((bucket_index + 1) * subBucketHalfCount) ):
    auto bucket_base_index = (bucket_index + 1) << sub_bucket_half_count_magnitude;
    // Calculate the offset in the bucket:
    auto offset_in_bucket = sub_bucket_index - sub_bucket_half_count;
    // The following is the equivalent of
    //  ((sub_bucket_index  - subBucketHalfCount) + bucketBaseIndex
    return bucket_base_index + offset_in_bucket;
}

std::int64_t HdrHistogram::hdr_size_of_equiv_value_range(std::int64_t value) const noexcept
{
    auto bucket_index = get_bucket_index(value);
    auto sub_bucket_index = get_sub_bucket_index(value, bucket_index);
    if (sub_bucket_index >= sub_bucket_count) {
        ++bucket_index;
    }
    return 1 << (unit_magnitude + bucket_index);
}

std::int64_t HdrHistogram::hdr_median_equiv_value(std::int64_t value) const noexcept
{
    return get_lowest_equivalent_value(value) + (hdr_size_of_equiv_value_range(value) >> 1);
}

std::ostream& operator<<(std::ostream& out, const HdrHistogram& hist)
{
    // TODO: Move to make_iterator functor
    const double output_value_unit_scaling_ratio = 1000.0;
    out << "       Value     Percentile TotalCount 1/(1-Percentile)\n\n";
    for (const auto& iter_value : hist) {
        const auto value = iter_value.value_iterated_to / output_value_unit_scaling_ratio;
        const auto percentile = iter_value.percentile_level_iterated_to / 100;
        const auto total_count = iter_value.total_count_to_this_value;

        // clang-format off
        out << std::setw(12) << std::defaultfloat << std::setprecision(12) << value
            << std::setw(15) << std::fixed << std::setprecision(12) << percentile
            << std::setw(11) << std::defaultfloat << total_count;
        // clang-format on

        if (iter_value.percentile_level_iterated_to != 100) {
            double other = 1.0 / (1.0 - iter_value.percentile_level_iterated_to / 100.0);
            out << std::setw(17) << std::fixed << std::setprecision(2) << other;
        }
        out << '\n';
    }

    const double mean = hist.mean() / output_value_unit_scaling_ratio;
    const double stddev = hist.stddev();
    const double max = hist.max() / output_value_unit_scaling_ratio;
    const double total = hist.get_total_count();

    // clang-format off
    return out <<
        "#[Mean    = " << std::setw(14) << std::setprecision(14) << mean << ", StdDeviation   = " << std::setprecision(13) << stddev << "]\n"
        "#[Max     = " << std::setw(14) << std::setprecision(14) << max << ", TotalCount     = " << total << "]\n"
        "#[Buckets = " << std::setw(14) << hist.bucket_count << ", SubBuckets     = " << hist.sub_bucket_count << "]\n";
    // clang-format on
}

} // namespace hdr
} // namespace toolbox
