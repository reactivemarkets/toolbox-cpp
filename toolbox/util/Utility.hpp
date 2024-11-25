// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_UTILITY_HPP
#define TOOLBOX_UTIL_UTILITY_HPP

#include <toolbox/Config.h>
#include <toolbox/util/Math.hpp>

#include <bit>
#include <cstdint>
#include <string_view>
#include <string>

namespace toolbox {
inline namespace util {

template <typename T>
struct AlwaysFalse : std::false_type {};

template <typename ValueT>
inline auto& remove_const(const ValueT& ref)
{
    return const_cast<ValueT&>(ref);
}

constexpr bool isdigit(int c) noexcept
{
    return c >= '0' && c <= '9';
}
static_assert(isdigit('0') && isdigit('9') && !isdigit('A'));

/// Returns the number of decimal digits in a unsigned integer.
///
/// \param i integer value.
/// \return the number of decimal digits.
template <typename ValueT>
    requires std::unsigned_integral<ValueT>
constexpr int dec_digits(ValueT i) noexcept {
    std::uint64_t v{i};

    // map v such that: even v --> v+1
    //                  odd  v --> v.
    //
    // The number of digits in v remains unchanged after this mapping.
    // However, the purpose of the mapping is to eliminate the need
    // for a branch to handle the case when v equals 0.
    v |= 1;
    [[assume(v != 0)]];

    // The number of digits in a *positive* base-10 number v is exactly:
    //      1+floor(log_2(v) / log_2(10))

    // 4096/1233 is a well known approximation to log_2(10).
    // (dividing by 4096 is equivalent to shifting right by 12).
    int digits = ((std::bit_width(v) * 1233) >> 12);

    // Due to the nature of approximations, the calculation is
    // of by 1 in the intervals: [10^k, next_pow2(10^k)).
    return digits + int{v >= pow10(digits)};
}

/// Returns the number of decimal digits in a signed integer.
///
/// \param i integer value.
/// \return the number of decimal digits.
template <typename ValueT>
    requires std::signed_integral<ValueT>
constexpr int dec_digits(ValueT i) noexcept {
    std::int64_t v{i};

    // abs(min value of int64_t) cannot be stored in int64_t
    // so handle it separately.
    if (v == std::numeric_limits<std::int64_t>::min()) [[unlikely]] {
        return 19;
    } else {
        std::int64_t abs_v = (v < 0) ? -v : v;
        return dec_digits(static_cast<std::uint64_t>(abs_v));
    }
}

/// Returns the number of hexadecimal digits in a positive integer.
///
/// \tparam UIntegerT Integer type.
/// \param i Integer value.
/// \return the number of hexadecimal digits.
/// \todo consider adding support for negative integers.
template <typename UIntegerT>
    requires std::unsigned_integral<UIntegerT>
constexpr int hex_digits(UIntegerT i) noexcept
{
    constexpr auto Bits = sizeof(i) * 8;
    return 1 + ((Bits - std::countl_zero(i | 1) - 1) >> 2);
}

static_assert(hex_digits(0x0U) == 1);
static_assert(hex_digits(0x1U) == 1);
static_assert(hex_digits(std::uint64_t{0xffffffffffff}) == 12);

TOOLBOX_API bool stob(std::string_view sv, bool dfl = false) noexcept;
TOOLBOX_API double stod(std::string_view sv, double dfl = {}) noexcept;

template <typename ValueT>
    requires std::integral<ValueT> || std::same_as<ValueT, double>
constexpr ValueT ston(std::string_view sv) noexcept
{
    if constexpr (std::is_same_v<ValueT, double>) {
        return stod(sv);
    } else if constexpr (std::is_integral_v<ValueT>) {
        auto it = sv.begin(), end = sv.end();
        if (it == end) {
            return 0;
        }
        bool neg{false};
        if constexpr (std::is_signed_v<ValueT>) {
            // Handle sign.
            if (*it == '-') {
                if (++it == end) {
                    return 0;
                }
                neg = true;
            }
        }
        std::uint64_t n{0};
        if (isdigit(*it)) {
            n = *it++ - '0';
            while (it != end && isdigit(*it)) {
                n *= 10;
                n += *it++ - '0';
            }
        }
        return neg ? -n : n;
    } else {
        static_assert(AlwaysFalse<ValueT>::value);
    }
}
static_assert(ston<int>(std::string_view{"-123"}) == -123);

struct string_hash {
    using is_transparent = void;
    std::size_t operator()(std::string_view txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    std::size_t operator()(const char* txt) const
    {
        return std::hash<std::string_view>{}(txt);
    }
    std::size_t operator()(const std::string& txt) const
    {
        return std::hash<std::string>{}(txt);
    }
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_UTILITY_HPP
