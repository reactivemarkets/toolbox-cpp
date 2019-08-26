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

#include "HdrHistogramIterator.hpp"
#include "HdrHistogram.hpp"

namespace toolbox {

inline namespace hdr {

using RecordedIteratorAdapter = IteratorAdapter<RecordedIterator>;

// Empty histogram for past the end iterator
static const HdrHistogram empty_hist{1, 1, 1};

AllValuesIterator::AllValuesIterator() = default;

AllValuesIterator::AllValuesIterator(const HdrHistogram& histogram)
: AbstractHdrIterator{histogram}
{
}

bool AllValuesIterator::reached_iteration_level() const
{
    return visited_index_ != this->current_index_;
}

void AllValuesIterator::increment_iteration_level()
{
    visited_index_ = this->current_index_;
}

bool AllValuesIterator::has_next()
{
    bool next = this->current_index_ < this->histogram_.counts_len - 1;
    if (!next) {
        this->end_ = true;
    }
    return next;
}

bool RecordedIterator::reached_iteration_level() const
{
    const auto current_count = this->histogram_.get_count_at_index(current_index_);
    return current_count and (visited_index_ != current_index_);
}

RecordedIterator::RecordedIterator() = default;

RecordedIterator::RecordedIterator(const HdrHistogram& histogram)
: AllValuesIterator{histogram}
{
}

bool operator==(const RecordedIterator& lhs, const RecordedIterator& rhs)
{
    return lhs.end_ == rhs.end_;
}

bool operator!=(const RecordedIterator& lhs, const RecordedIterator& rhs)
{
    return !(lhs == rhs);
}

/// End iterators

HdrIterationValue::HdrIterationValue(const AbstractHdrIterator& iterator)
: iterator_{iterator}
{
}

void HdrIterationValue::set(std::int64_t value) noexcept
{
    value_iterated_to = value;
    value_iterated_from = iterator_.prev_value_iterated_to_;
    count_at_value_iterated_to = iterator_.count_at_this_value_;
    count_added_in_this_iter_step
        = iterator_.total_count_to_current_index_ - iterator_.total_count_to_prev_index_;
    total_count_to_this_value = iterator_.total_count_to_current_index_;
    total_value_to_this_value = iterator_.value_to_index_;
    percentile = (100.0 * iterator_.total_count_to_current_index_) / iterator_.total_count_;
    percentile_level_iterated_to = iterator_.get_percentile_iterated_to();
    int_to_double_conversion_ratio = iterator_.int_to_double_conversion_ratio_;
}

AbstractHdrIterator::AbstractHdrIterator()
: histogram_{empty_hist}
, current_iteration_value_{*this}
, end_{true}
{
}

AbstractHdrIterator::AbstractHdrIterator(const HdrHistogram& histogram)
: histogram_{histogram} //         , total_count_to_current_index_{total_count_to_current_index}
, value_at_next_index_{1ll << histogram.unit_magnitude}
, current_iteration_value_{*this}
, total_count_{histogram.total_count_}
, int_to_double_conversion_ratio_{histogram.int_to_double_conversion_ratio_}
{
}

AbstractHdrIterator::~AbstractHdrIterator() = default;

AbstractHdrIterator& AbstractHdrIterator::operator++()
{
    while (has_next()) {
        count_at_this_value_ = histogram_.get_count_at_index(current_index_);
        if (fresh_sub_bucket_) {
            total_count_to_current_index_ += count_at_this_value_;
            value_to_index_ += count_at_this_value_ * get_value_iterated_to();
            fresh_sub_bucket_ = false;
        }
        if (reached_iteration_level()) {
            auto value_iterated_to = get_value_iterated_to();
            current_iteration_value_.set(value_iterated_to);

            prev_value_iterated_to_ = value_iterated_to;
            total_count_to_prev_index_ = total_count_to_current_index_;

            increment_iteration_level();

            return *this;
        }

        increment_sub_bucket();
    }
    return *this;
}

void AbstractHdrIterator::increment_sub_bucket()
{
    fresh_sub_bucket_ = true;
    ++current_index_;
    value_at_index_ = histogram_.get_value_from_index(current_index_);
    value_at_next_index_ = histogram_.get_value_from_index(current_index_ + 1);
}

bool AbstractHdrIterator::has_next()
{
    return total_count_to_current_index_ < total_count_;
}

std::int64_t AbstractHdrIterator::get_count_at_this_value() const noexcept
{
    return count_at_this_value_;
}

double AbstractHdrIterator::get_percentile_iterated_to() const noexcept
{
    return (100.0 * total_count_to_current_index_) / total_count_;
}

double AbstractHdrIterator::get_percentile_iterated_from() const noexcept
{
    return (100.0 * total_count_to_prev_index_) / total_count_;
}

std::int64_t AbstractHdrIterator::get_value_iterated_to() const
{
    return histogram_.get_highest_equivalent_value(value_at_index_);
}

PercentileIterator::PercentileIterator() = default;

PercentileIterator::PercentileIterator(const HdrHistogram& histogram,
                                       double percentile_ticks_per_half_distance)
: AbstractHdrIterator{histogram}
, percentile_ticks_per_half_distance_{percentile_ticks_per_half_distance}
{
}

const HdrIterationValue& AbstractHdrIterator::operator*() const noexcept
{
    return this->current_iteration_value_;
}
const HdrIterationValue* AbstractHdrIterator::operator->() const noexcept
{
    return &this->current_iteration_value_;
}

bool PercentileIterator::has_next()
{
    if (AbstractHdrIterator::has_next()) {
        return true;
    }

    // We want one additional last step to 100%
    if (!reached_last_recorded_value_ && this->total_count_) {
        percentile_to_iterate_to_ = 100.0;
        reached_last_recorded_value_ = true;
        return true;
    }

    this->end_ = true;
    return false;
}

void PercentileIterator::increment_iteration_level()
{
    this->percentile_to_iterate_from_ = this->percentile_to_iterate_to_;
    auto percentile_gap{100.0 - this->percentile_to_iterate_to_};
    if (percentile_gap) {
        auto half_distance = std::pow(2, (std::log(100 / percentile_gap) / std::log(2)) + 1);
        auto percentile_reporting_ticks = percentile_ticks_per_half_distance_ * half_distance;
        this->percentile_to_iterate_to_ += 100.0 / percentile_reporting_ticks;
    }
}

bool PercentileIterator::reached_iteration_level() const
{
    if (this->count_at_this_value_ == 0) {
        return false;
    }
    auto current_percentile = (100.0 * this->total_count_to_current_index_) / this->total_count_;
    return current_percentile >= this->percentile_to_iterate_to_;
}

double PercentileIterator::get_percentile_iterated_to() const noexcept
{
    return this->percentile_to_iterate_to_;
}

double PercentileIterator::get_percentile_iterated_from() const noexcept
{
    return this->percentile_to_iterate_from_;
}

PercentileIterator HdrHistogram::begin() const
{
    PercentileIterator p{*this};
    // TODO port leftover, having to inc manually
    ++p;
    return p;
}

PercentileIterator HdrHistogram::end() const
{
    return {};
}

bool operator==(const PercentileIterator& lhs, const PercentileIterator& rhs)
{
    return lhs.end_ == rhs.end_;
}

bool operator!=(const PercentileIterator& lhs, const PercentileIterator& rhs)
{
    return !(lhs == rhs);
}

} // namespace hdr
} // namespace toolbox
