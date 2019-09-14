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
#ifndef TOOLBOX_HDR_ITERATOR
#define TOOLBOX_HDR_ITERATOR

#include <toolbox/Config.h>

#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <vector>

namespace toolbox {
inline namespace hdr {

class HdrHistogram;
class HdrIterator;

class TOOLBOX_API HdrIterationValue {
  public:
    HdrIterationValue(const HdrIterator& iterator);

    void set(std::int64_t value) noexcept;

    std::int64_t value_iterated_to{0};
    std::int64_t value_iterated_from{0};
    std::int64_t count_at_value_iterated_to{0};
    std::int64_t count_added_in_this_iter_step{0};
    std::int64_t total_count_to_this_value{0};
    std::int64_t total_value_to_this_value{0};
    double percentile{0.0};
    double percentile_level_iterated_to{0.0};
    double int_to_double_conversion_ratio{1.0};

  private:
    const HdrIterator& iterator_;
};

class TOOLBOX_API HdrIterator {
  public:
    explicit HdrIterator(const HdrHistogram& hist);

    /// Past the end iterator.
    HdrIterator();
    virtual ~HdrIterator();

    virtual bool has_next() const noexcept;
    virtual bool reached_iteration_level() const = 0;
    virtual double get_percentile_iterated_to() const noexcept;
    virtual double get_percentile_iterated_from() const noexcept;

    std::int64_t get_value_iterated_to() const noexcept;
    std::int64_t get_count_at_this_value() const noexcept;
    const HdrIterationValue& operator*() const noexcept;
    const HdrIterationValue* operator->() const noexcept;

    virtual void increment_iteration_level() = 0;
    HdrIterator& operator++();
    void increment_sub_bucket();

  protected:
    friend class HdrIterationValue;

    const HdrHistogram& hist_;
    std::int64_t current_index_{0};
    std::int64_t count_at_this_value_{0};
    std::int64_t total_count_to_current_index_{0};
    std::int64_t total_count_to_prev_index_{0};
    std::int64_t prev_value_iterated_to_{0};
    std::int64_t value_at_index_{0};
    std::int64_t value_to_index_{0};
    std::int64_t value_at_next_index_{0};
    HdrIterationValue current_iteration_value_;
    std::int64_t total_count_;
    bool fresh_sub_bucket_{true};
    mutable bool end_{false};
    double int_to_double_conversion_ratio_{1.0};
};

class TOOLBOX_API AllValuesIterator : public HdrIterator {
  public:
    explicit AllValuesIterator(const HdrHistogram& hist);
    // Past the end iterator.
    AllValuesIterator();

    bool has_next() const noexcept override;
    bool reached_iteration_level() const override;
    void increment_iteration_level() override;

  protected:
    std::int64_t visited_index_{-1};
};

/// Provide a means of iterating through all recorded histogram values
/// using the finest granularity steps supported by the underlying representation.
/// The iteration steps through all non-zero recorded value counts,
/// and terminates when all recorded histogram values are exhausted.
class TOOLBOX_API RecordedIterator : public AllValuesIterator {
    friend bool operator==(const RecordedIterator& lhs, const RecordedIterator& rhs) noexcept;
    friend bool operator!=(const RecordedIterator& lhs, const RecordedIterator& rhs) noexcept;

  public:
    explicit RecordedIterator(const HdrHistogram& hist);
    /// Past the end iterator.
    RecordedIterator();

    bool reached_iteration_level() const override;
};

template <typename IteratorT>
class IteratorAdapter {
  public:
    explicit IteratorAdapter(const HdrHistogram& hist)
    : hist_{hist}
    {
    }
    IteratorT begin() const
    {
        auto it = IteratorT{hist_};
        ++it;
        return it;
    }
    IteratorT end() const { return {}; }

  private:
    const HdrHistogram& hist_;
};

class TOOLBOX_API PercentileIterator : public HdrIterator {
    friend bool operator==(const PercentileIterator& lhs, const PercentileIterator& rhs) noexcept;
    friend bool operator!=(const PercentileIterator& lhs, const PercentileIterator& rhs) noexcept;

  public:
    explicit PercentileIterator(const HdrHistogram& hist,
                                double percentile_ticks_per_half_distance = 5.0);
    /// Past the end iterator.
    PercentileIterator();

    bool has_next() const noexcept override;
    bool reached_iteration_level() const noexcept override;
    double get_percentile_iterated_to() const noexcept override;
    double get_percentile_iterated_from() const noexcept override;

    void increment_iteration_level() override;

  private:
    double percentile_ticks_per_half_distance_;
    mutable bool reached_last_recorded_value_{false};
    mutable double percentile_to_iterate_to_{0.0};
    double percentile_to_iterate_from_{0.0};
};

} // namespace hdr
} // namespace toolbox

#endif // TOOLBOX_HDR_ITERATOR
