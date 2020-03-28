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

#ifndef TOOLBOX_UTIL_COMPARE_HPP
#define TOOLBOX_UTIL_COMPARE_HPP

#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename ValueT>
struct Comparable {
    friend constexpr bool operator==(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) == 0;
    }

    friend constexpr bool operator!=(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) != 0;
    }

    friend constexpr bool operator<(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) < 0;
    }

    friend constexpr bool operator<=(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) <= 0;
    }

    friend constexpr bool operator>(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) > 0;
    }

    friend constexpr bool operator>=(const ValueT& lhs, const ValueT& rhs) noexcept
    {
        return lhs.compare(rhs) >= 0;
    }

  protected:
    constexpr Comparable() noexcept = default;
    ~Comparable() = default;

    // Copy.
    constexpr Comparable(const Comparable&) noexcept = default;
    Comparable& operator=(const Comparable&) noexcept = default;

    // Move.
    constexpr Comparable(Comparable&&) noexcept = default;
    Comparable& operator=(Comparable&&) noexcept = default;
};

template <typename ValueT>
constexpr int compare(ValueT lhs, ValueT rhs) noexcept
{
    int i{};
    if (lhs < rhs) {
        i = -1;
    } else if (lhs > rhs) {
        i = 1;
    } else {
        i = 0;
    }
    return i;
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_COMPARE_HPP
