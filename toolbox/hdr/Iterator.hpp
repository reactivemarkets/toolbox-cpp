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
#ifndef TOOLBOX_HDR_ITERATOR
#define TOOLBOX_HDR_ITERATOR

#include <toolbox/Config.h>

#include <cstdint>

namespace toolbox {
/// A C++ port of HdrHistogram_c written Michael Barker and released to the public domain.
inline namespace hdr {
class HdrHistogram;

/// HdrIterator is the base iterator for all iterator types.
class TOOLBOX_API HdrIterator {
  public:
    /// Construct iterator.
    ///
    /// \param h The histogram to iterate over.
    HdrIterator(const HdrHistogram& h) noexcept;
    virtual ~HdrIterator();

    // Copy.
    HdrIterator(const HdrIterator&) = delete;
    HdrIterator& operator=(const HdrIterator&) = delete;

    // Move.
    HdrIterator(HdrIterator&&) noexcept = default;
    HdrIterator& operator=(HdrIterator&&) = delete;

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

    const HdrHistogram& h_;
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

/// HdrPercentileIterator is a percentile iterator.
class TOOLBOX_API HdrPercentileIterator : public HdrIterator {
  public:
    HdrPercentileIterator(const HdrHistogram& h, std::int32_t ticks_per_half_distance) noexcept;
    ~HdrPercentileIterator() final = default;

    // Copy.
    HdrPercentileIterator(const HdrPercentileIterator&) = delete;
    HdrPercentileIterator& operator=(const HdrPercentileIterator&) = delete;

    // Move.
    HdrPercentileIterator(HdrPercentileIterator&&) noexcept = default;
    HdrPercentileIterator& operator=(HdrPercentileIterator&&) = delete;

    double percentile() const noexcept { return percentile_; }

  protected:
    bool do_next() noexcept final;

  private:
    bool seen_last_value_;
    std::int32_t ticks_per_half_distance_;
    double percentile_to_iterate_to_;
    double percentile_;
};

/// HdrCountAddedIterator is a recorded value iterator.
class TOOLBOX_API HdrCountAddedIterator : public HdrIterator {
  public:
    // Copy.
    HdrCountAddedIterator(const HdrCountAddedIterator&) = delete;
    HdrCountAddedIterator& operator=(const HdrCountAddedIterator&) = delete;

    // Move.
    HdrCountAddedIterator(HdrCountAddedIterator&&) noexcept = default;
    HdrCountAddedIterator& operator=(HdrCountAddedIterator&&) = delete;

    std::int64_t count_added_in_this_iteration_step() const noexcept
    {
        return count_added_in_this_iteration_step_;
    }

  protected:
    explicit HdrCountAddedIterator(const HdrHistogram& h);
    ~HdrCountAddedIterator() override;

    std::int64_t count_added_in_this_iteration_step_{0};
};

/// HdrRecordedIterator is a recorded value iterator.
class TOOLBOX_API HdrRecordedIterator : public HdrCountAddedIterator {
  public:
    HdrRecordedIterator(const HdrHistogram& h);
    ~HdrRecordedIterator() final = default;

    // Copy.
    HdrRecordedIterator(const HdrRecordedIterator&) = delete;
    HdrRecordedIterator& operator=(const HdrRecordedIterator&) = delete;

    // Move.
    HdrRecordedIterator(HdrRecordedIterator&&) noexcept = default;
    HdrRecordedIterator& operator=(HdrRecordedIterator&&) = delete;

  protected:
    bool do_next() noexcept final;
};

/// HdrLinearIterator is a linear value iterator.
class TOOLBOX_API HdrLinearIterator : public HdrCountAddedIterator {
  public:
    HdrLinearIterator(const HdrHistogram& h, std::int64_t value_units_per_bucket) noexcept;
    ~HdrLinearIterator() final = default;

    // Copy.
    HdrLinearIterator(const HdrLinearIterator&) = delete;
    HdrLinearIterator& operator=(const HdrLinearIterator&) = delete;

    // Move.
    HdrLinearIterator(HdrLinearIterator&&) noexcept = default;
    HdrLinearIterator& operator=(HdrLinearIterator&&) = delete;

  protected:
    bool do_next() noexcept final;

  private:
    std::int64_t value_units_per_bucket_;
    std::int64_t next_value_reporting_level_;
    std::int64_t next_value_reporting_level_lowest_equivalent_;
};

/// HdrLogIterator is a logarithmic value iterator.
class TOOLBOX_API HdrLogIterator : public HdrCountAddedIterator {
  public:
    HdrLogIterator(const HdrHistogram& h, std::int64_t value_units_first_bucket,
                   double log_base) noexcept;
    ~HdrLogIterator() final = default;

    // Copy.
    HdrLogIterator(const HdrLogIterator&) = delete;
    HdrLogIterator& operator=(const HdrLogIterator&) = delete;

    // Move.
    HdrLogIterator(HdrLogIterator&&) noexcept = default;
    HdrLogIterator& operator=(HdrLogIterator&&) = delete;

  protected:
    bool do_next() noexcept final;

  private:
    double log_base_;
    std::int64_t next_value_reporting_level_;
    std::int64_t next_value_reporting_level_lowest_equivalent_;
};

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_ITERATOR
