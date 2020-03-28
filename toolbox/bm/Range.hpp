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

#ifndef TOOLBOX_BM_RANGE
#define TOOLBOX_BM_RANGE

#include <toolbox/Config.h>

#include <chrono>

namespace toolbox {
inline namespace hdr {
class HdrHistogram;
} // namespace hdr
} // namespace toolbox

namespace toolbox::bm {

/// The BenchmarkRange class records the time elapsed during object lifetime,
/// i.e., between construction and destruction.
/// The elapsed time is recorded in the HdrHistogram object during destruction.
class TOOLBOX_API BenchmarkRange {
    class Iterator {
        friend constexpr bool operator==(Iterator lhs, Iterator rhs) noexcept
        {
            return lhs.val_ == rhs.val_;
        }
        friend constexpr bool operator!=(Iterator lhs, Iterator rhs) noexcept
        {
            return !(lhs == rhs);
        }

      public:
        constexpr Iterator(int val) noexcept
        : val_{val}
        {
        }
        constexpr auto operator*() const noexcept { return val_; }
        Iterator& operator++() noexcept
        {
            ++val_;
            return *this;
        }

      private:
        int val_{0};
    };

  public:
    BenchmarkRange(HdrHistogram& hist, int first, int last) noexcept;
    ~BenchmarkRange();

    // Copy.
    BenchmarkRange(const BenchmarkRange&) = delete;
    BenchmarkRange& operator=(const BenchmarkRange&) = delete;

    // Move.
    BenchmarkRange(BenchmarkRange&&) = delete;
    BenchmarkRange& operator=(BenchmarkRange&&) = delete;

    auto begin() const noexcept { return Iterator{first_}; }
    auto end() const noexcept { return Iterator{last_}; }

  private:
    HdrHistogram& hist_;
    const int first_;
    const int last_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace toolbox::bm

#endif // TOOLBOX_BM_RANGE
