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

#include "Iterator.hpp"

#include "Histogram.hpp"

#include <cmath>

namespace toolbox {
inline namespace hdr {
using namespace std;

HdrIterator::HdrIterator(const HdrHistogram& h) noexcept
: h_{h}
, counts_index_{-1}
, total_count_{h.total_count()}
, count_{0}
, cumulative_count_{0}
, value_{0}
, highest_equivalent_value_{0}
, value_iterated_from_{0}
, value_iterated_to_{0}
{
}

HdrIterator::~HdrIterator() = default;

bool HdrIterator::do_next() noexcept
{
    bool result = move_next();
    if (result) {
        update_iterated_values(value_);
    }
    return result;
}

bool HdrIterator::has_buckets() const noexcept
{
    return counts_index_ < h_.counts_len();
}

bool HdrIterator::has_next() const noexcept
{
    return cumulative_count_ < total_count_;
}

int64_t HdrIterator::peek_next_value_from_index() const noexcept
{
    return h_.value_at_index(counts_index_ + 1);
}

bool HdrIterator::next_value_greater_than_reporting_level_upper_bound(
    int64_t reporting_level_upper_bound) const noexcept
{
    if (counts_index_ >= h_.counts_len()) {
        return false;
    }
    return peek_next_value_from_index() > reporting_level_upper_bound;
}

bool HdrIterator::basic_next() noexcept
{
    if (!has_next() || counts_index_ >= h_.counts_len()) {
        return false;
    }
    move_next();
    return true;
}

bool HdrIterator::move_next() noexcept
{
    counts_index_++;

    if (!has_buckets()) {
        return false;
    }

    count_ = h_.counts_get_normalised(counts_index_);
    cumulative_count_ += count_;

    value_ = h_.value_at_index(counts_index_);
    highest_equivalent_value_ = h_.highest_equivalent_value(value_);
    lowest_equivalent_value_ = h_.lowest_equivalent_value(value_);
    median_equivalent_value_ = h_.median_equivalent_value(value_);
    return true;
}

void HdrIterator::update_iterated_values(int64_t new_value_iterated_to) noexcept
{
    value_iterated_from_ = value_iterated_to_;
    value_iterated_to_ = new_value_iterated_to;
}

HdrPercentileIterator::HdrPercentileIterator(const HdrHistogram& h,
                                             int32_t ticks_per_half_distance) noexcept
: HdrIterator{h}
, seen_last_value_{false}
, ticks_per_half_distance_{ticks_per_half_distance}
, percentile_to_iterate_to_{0.0}
, percentile_{0.0}
{
}

bool HdrPercentileIterator::do_next() noexcept
{
    if (!has_next()) {
        if (seen_last_value_) {
            return false;
        }
        seen_last_value_ = true;
        percentile_ = 100.0;
        return true;
    }

    if (counts_index_ == -1 && !basic_next()) {
        return false;
    }
    do {
        const double current_percentile{cumulative_count_ * 100.0 / h_.total_count()};
        if (count_ != 0 && percentile_to_iterate_to_ <= current_percentile) {
            update_iterated_values(h_.highest_equivalent_value(value_));

            percentile_ = percentile_to_iterate_to_;
            const int64_t temp = (log(100.0 / (100.0 - percentile_to_iterate_to_)) / log(2)) + 1.0;
            const int64_t half_distance = pow(2, static_cast<double>(temp));
            const int64_t percentile_reporting_ticks{ticks_per_half_distance_ * half_distance};
            percentile_to_iterate_to_ += 100.0 / percentile_reporting_ticks;

            return true;
        }
    } while (basic_next());

    return true;
}

HdrCountAddedIterator::HdrCountAddedIterator(const HdrHistogram& h)
: HdrIterator{h}
{
}

HdrCountAddedIterator::~HdrCountAddedIterator() = default;

HdrRecordedIterator::HdrRecordedIterator(const HdrHistogram& h)
: HdrCountAddedIterator{h}
{
}

bool HdrRecordedIterator::do_next() noexcept
{
    while (basic_next()) {
        if (count_ != 0) {
            update_iterated_values(value_);
            count_added_in_this_iteration_step_ = count_;
            return true;
        }
    }
    return false;
}

HdrLinearIterator::HdrLinearIterator(const HdrHistogram& h, int64_t value_units_per_bucket) noexcept
: HdrCountAddedIterator{h}
, value_units_per_bucket_{value_units_per_bucket}
, next_value_reporting_level_{value_units_per_bucket}
, next_value_reporting_level_lowest_equivalent_{h.lowest_equivalent_value(value_units_per_bucket)}
{
}

bool HdrLinearIterator::do_next() noexcept
{
    count_added_in_this_iteration_step_ = 0;

    if (has_next()
        || next_value_greater_than_reporting_level_upper_bound(
            next_value_reporting_level_lowest_equivalent_)) {
        do {
            if (value_ >= next_value_reporting_level_lowest_equivalent_) {
                update_iterated_values(next_value_reporting_level_);

                next_value_reporting_level_ += value_units_per_bucket_;
                next_value_reporting_level_lowest_equivalent_
                    = h_.lowest_equivalent_value(next_value_reporting_level_);

                return true;
            }
            if (!move_next()) {
                return true;
            }
            count_added_in_this_iteration_step_ += count_;
        } while (true);
    }
    return false;
}

HdrLogIterator::HdrLogIterator(const HdrHistogram& h, int64_t value_units_first_bucket,
                               double log_base) noexcept
: HdrCountAddedIterator{h}
, log_base_{log_base}
, next_value_reporting_level_{value_units_first_bucket}
, next_value_reporting_level_lowest_equivalent_{h.lowest_equivalent_value(value_units_first_bucket)}
{
}

bool HdrLogIterator::do_next() noexcept
{
    count_added_in_this_iteration_step_ = 0;

    if (has_next()
        || next_value_greater_than_reporting_level_upper_bound(
            next_value_reporting_level_lowest_equivalent_)) {
        do {
            if (value_ >= next_value_reporting_level_lowest_equivalent_) {
                update_iterated_values(next_value_reporting_level_);

                next_value_reporting_level_ *= log_base_;
                next_value_reporting_level_lowest_equivalent_
                    = h_.lowest_equivalent_value(next_value_reporting_level_);

                return true;
            }
            if (!move_next()) {
                return true;
            }
            count_added_in_this_iteration_step_ += count_;
        } while (true);
    }
    return false;
}

} // namespace hdr
} // namespace toolbox
