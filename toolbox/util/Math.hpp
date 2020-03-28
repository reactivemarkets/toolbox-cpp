// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#ifndef TOOLBOX_UTIL_MATH_HPP
#define TOOLBOX_UTIL_MATH_HPP

#include <toolbox/util/Bits.hpp>

#include <toolbox/Config.h>

#include <algorithm>
#include <cmath>

namespace toolbox {
inline namespace util {

/// \return true if size is a power of two.
constexpr bool is_pow2(std::size_t n) noexcept
{
    return n > 0 && (n & (n - 1)) == 0;
}

/// \return the next power of two.
inline unsigned next_pow2(unsigned n) noexcept
{
    // The result of clz is undefined for zero values.
    return n <= 1 ? 1 : 1 << (sizeof(n) * 8 - clz(n - 1));
}

/// \return the next power of two.
inline unsigned long next_pow2(unsigned long n) noexcept
{
    // The result of clz is undefined for zero values.
    return n <= 1 ? 1 : 1 << (sizeof(n) * 8 - clz(n - 1));
}

template <int BitsN>
constexpr std::size_t ceil_pow2(std::size_t size) noexcept
{
    enum { Max = (1 << BitsN) - 1 };
    return ((size + Max) >> BitsN) << BitsN;
}

class TOOLBOX_API VarAccum {
  public:
    constexpr VarAccum() noexcept = default;
    ~VarAccum() = default;

    // Copy.
    constexpr VarAccum(const VarAccum&) noexcept = default;
    VarAccum& operator=(const VarAccum&) noexcept = default;

    // Move.
    constexpr VarAccum(VarAccum&&) noexcept = default;
    VarAccum& operator=(VarAccum&&) noexcept = default;

    bool empty() const noexcept { return size_ == 0; }
    std::size_t size() const noexcept { return size_; }
    double mean() const noexcept { return mean_; }
    double sum2() const noexcept { return sum2_; }
    double min() const noexcept { return min_; }
    double max() const noexcept { return max_; }

    void clear() noexcept
    {
        size_ = 0;
        mean_ = 0.0;
        sum2_ = 0.0;
        min_ = std::numeric_limits<double>::max();
        max_ = std::numeric_limits<double>::min();
    }
    void append(double val) noexcept
    {
        ++size_;
        double delta{val - mean_};
        mean_ += delta / size_;
        sum2_ += delta * (val - mean_);
        min_ = std::min(min_, val);
        max_ = std::max(max_, val);
    }
    template <typename... ArgsT>
    void append(double first, ArgsT... args) noexcept
    {
        append(first);
        // Recursively apply to tail.
        append(args...);
    }

  private:
    std::size_t size_{0};
    double mean_{0.0};
    double sum2_{0.0};
    double min_{std::numeric_limits<double>::max()};
    double max_{std::numeric_limits<double>::min()};
};

inline double var(const VarAccum& v) noexcept
{
    return v.size() > 1 ? v.sum2() / (v.size() - 1) : std::numeric_limits<double>::quiet_NaN();
}

inline double varp(const VarAccum& v) noexcept
{
    return !v.empty() ? v.sum2() / v.size() : std::numeric_limits<double>::quiet_NaN();
}

inline double stdev(const VarAccum& v) noexcept
{
    return std::sqrt(var(v));
}

inline double stdevp(const VarAccum& v) noexcept
{
    return std::sqrt(varp(v));
}

inline double zscore(double mean, double sd, double val) noexcept
{
    return (val - mean) / sd;
}

inline double pctile95(double mean, double sd) noexcept
{
    // NORMSINV(0.95) = 1.6448536
    return mean + 1.6448536 * sd;
}

inline double pctile99(double mean, double sd) noexcept
{
    // NORMSINV(0.99) = 2.3263479
    return mean + 2.3263479 * sd;
}

inline double pctile999(double mean, double sd) noexcept
{
    // NORMSINV(0.999) = 3.0902323
    return mean + 3.0902323 * sd;
}

/// \return the ceiling of dividend / divisor.
constexpr std::size_t ceil(std::size_t dividend, std::size_t divisor) noexcept
{
    return (dividend - 1) / divisor + 1;
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_MATH_HPP
