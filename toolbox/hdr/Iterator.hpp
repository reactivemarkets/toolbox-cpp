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
#ifndef TOOLBOX_HDR_ITERATOR
#define TOOLBOX_HDR_ITERATOR

#include <toolbox/Config.h>

#include <cstdint>

namespace toolbox {
/// A C++ port of HdrHistogram_c written Michael Barker and released to the public domain.
inline namespace hdr {
class Histogram;

/// Iterator is the base iterator for all iterator types.
class TOOLBOX_API Iterator {
  public:
    /// Construct iterator.
    ///
    /// \param h The histogram to iterate over.
    explicit Iterator(const Histogram& h) noexcept;
    virtual ~Iterator();

    // Copy.
    Iterator(const Iterator&) = delete;
    Iterator& operator=(const Iterator&) = delete;

    // Move.
    Iterator(Iterator&&) noexcept = default;
    Iterator& operator=(Iterator&&) = delete;

    /// Value directly from array for the current counts_index.
    std::int64_t count() const noexcept { return count_; }
    /// Sum of all of the counts up to and including the count at this index.
    std::int64_t cumulative_count() const noexcept { return cumulative_count_; }
    /// The current value based on counts_index.
    std::int64_t value() const noexcept { return value_; }
    std::int64_t highest_equivalent_value() const noexcept { return highest_equivalent_value_; }

    /// Iterate to the next value for the iterator. If there are no more values available return
    /// false.
    ///
    /// \return false if there are no values remaining for this iterator.
    bool next() noexcept { return do_next(); }

  protected:
    bool has_buckets() const noexcept;
    bool has_next() const noexcept;
    std::int64_t peek_next_value_from_index() const noexcept;
    bool next_value_greater_than_reporting_level_upper_bound(
        std::int64_t reporting_level_upper_bound) const noexcept;

    bool basic_next() noexcept;
    bool move_next() noexcept;
    void update_iterated_values(std::int64_t new_value_iterated_to) noexcept;

    virtual bool do_next() noexcept;

    const Histogram& h_;
    /// Raw index into the counts array.
    std::int32_t counts_index_;
    /// Snapshot of the length at the time the iterator is created.
    std::int64_t total_count_;
    std::int64_t count_;
    std::int64_t cumulative_count_;
    std::int64_t value_;
    std::int64_t highest_equivalent_value_;
    std::int64_t lowest_equivalent_value_;
    std::int64_t median_equivalent_value_;
    std::int64_t value_iterated_from_;
    std::int64_t value_iterated_to_;
};

/// PercentileIterator is a percentile iterator.
class TOOLBOX_API PercentileIterator final : public Iterator {
  public:
    PercentileIterator(const Histogram& h, std::int32_t ticks_per_half_distance) noexcept;
    ~PercentileIterator() override = default;

    // Copy.
    PercentileIterator(const PercentileIterator&) = delete;
    PercentileIterator& operator=(const PercentileIterator&) = delete;

    // Move.
    PercentileIterator(PercentileIterator&&) noexcept = default;
    PercentileIterator& operator=(PercentileIterator&&) = delete;

    double percentile() const noexcept { return percentile_; }

  protected:
    bool do_next() noexcept override;

  private:
    bool seen_last_value_;
    std::int32_t ticks_per_half_distance_;
    double percentile_to_iterate_to_;
    double percentile_;
};

/// CountAddedIterator is a recorded value iterator.
class TOOLBOX_API CountAddedIterator : public Iterator {
  public:
    // Copy.
    CountAddedIterator(const CountAddedIterator&) = delete;
    CountAddedIterator& operator=(const CountAddedIterator&) = delete;

    // Move.
    CountAddedIterator(CountAddedIterator&&) noexcept = default;
    CountAddedIterator& operator=(CountAddedIterator&&) = delete;

    std::int64_t count_added_in_this_iteration_step() const noexcept
    {
        return count_added_in_this_iteration_step_;
    }

  protected:
    explicit CountAddedIterator(const Histogram& h);
    ~CountAddedIterator() override;

    std::int64_t count_added_in_this_iteration_step_{0};
};

/// RecordedIterator is a recorded value iterator.
class TOOLBOX_API RecordedIterator final : public CountAddedIterator {
  public:
    explicit RecordedIterator(const Histogram& h);
    ~RecordedIterator() override = default;

    // Copy.
    RecordedIterator(const RecordedIterator&) = delete;
    RecordedIterator& operator=(const RecordedIterator&) = delete;

    // Move.
    RecordedIterator(RecordedIterator&&) noexcept = default;
    RecordedIterator& operator=(RecordedIterator&&) = delete;

  protected:
    bool do_next() noexcept override;
};

/// LinearIterator is a linear value iterator.
class TOOLBOX_API LinearIterator final : public CountAddedIterator {
  public:
    LinearIterator(const Histogram& h, std::int64_t value_units_per_bucket) noexcept;
    ~LinearIterator() override = default;

    // Copy.
    LinearIterator(const LinearIterator&) = delete;
    LinearIterator& operator=(const LinearIterator&) = delete;

    // Move.
    LinearIterator(LinearIterator&&) noexcept = default;
    LinearIterator& operator=(LinearIterator&&) = delete;

  protected:
    bool do_next() noexcept override;

  private:
    std::int64_t value_units_per_bucket_;
    std::int64_t next_value_reporting_level_;
    std::int64_t next_value_reporting_level_lowest_equivalent_;
};

/// LogIterator is a logarithmic value iterator.
class TOOLBOX_API LogIterator final : public CountAddedIterator {
  public:
    LogIterator(const Histogram& h, std::int64_t value_units_first_bucket,
                double log_base) noexcept;
    ~LogIterator() override = default;

    // Copy.
    LogIterator(const LogIterator&) = delete;
    LogIterator& operator=(const LogIterator&) = delete;

    // Move.
    LogIterator(LogIterator&&) noexcept = default;
    LogIterator& operator=(LogIterator&&) = delete;

  protected:
    bool do_next() noexcept override;

  private:
    double log_base_;
    std::int64_t next_value_reporting_level_;
    std::int64_t next_value_reporting_level_lowest_equivalent_;
};

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_ITERATOR
