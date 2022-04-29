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
#include <toolbox/util/Concepts.hpp>

#include <bit>
#include <cstdint>
#include <string_view>
#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename T>
struct AlwaysFalse : std::false_type {
};

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

/// Returns the number of decimal digits in a positive, signed integer.
///
/// \param i Integer value.
/// \return the number of decimal digits.
/// \todo consider adding support for negative integers.
constexpr int dec_digits(std::int64_t i) noexcept
{
    return i < 10000000000        ? i < 100000 ? i < 100 ? i < 10 ? 1 : 2
                       : i < 1000                        ? 3
                       : i < 10000                       ? 4
                                                         : 5
                   : i < 10000000              ? i < 1000000 ? 6 : 7
                   : i < 100000000             ? 8
                   : i < 1000000000            ? 9
                                               : 10
        : i < 1000000000000000    ? i < 1000000000000 ? i < 100000000000 ? 11 : 12
               : i < 10000000000000                   ? 13
               : i < 100000000000000                  ? 14
                                                      : 15
        : i < 100000000000000000  ? i < 10000000000000000 ? 16 : 17
        : i < 1000000000000000000 ? 18
                                  : 19;
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
TOOLBOX_API double stod(std::string_view sv) noexcept;

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

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_UTILITY_HPP
