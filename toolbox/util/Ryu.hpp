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

#ifndef TOOLBOX_UTIL_RYU_HPP
#define TOOLBOX_UTIL_RYU_HPP

#include <toolbox/Config.h>

#include <string_view>

namespace toolbox {
inline namespace util {

/// Maximum precision supported Ryu fixed formatted doubles.
constexpr int MaxRyuPrec{308};

/// Calculates the maximum buffer space required for Ryu fixed formatted doubles.
/// This upper bound is calculated as follows:
/// - 1 character for sign;
/// - 20 digits for max unsigned 64-bit integer;
/// - 1 character for decimal point;
/// - max precision digits.
constexpr std::size_t max_ryu_fixed_buf(int prec = MaxRyuPrec) noexcept
{
    return 1 + 20 + 1 + prec;
}

/// Maximum buffer space require for Ryu formatted doubles.
/// This upper bound is chosen based on the internal buffer allocation within the d2s function.
/// See d2s.c for details.
constexpr std::size_t MaxRyuDtosBuf{24};

/// Maximum buffer space require for Ryu fixed formatted doubles.
constexpr std::size_t MaxRyuFixedBuf{max_ryu_fixed_buf()};

/// Convert double to string.
/// Returns the size of the resulting string.
/// The resulting string is not null terminated.
TOOLBOX_API std::size_t dtos(char* dst, double d) noexcept;

/// Convert double to string.
/// Returns a string_view of the resulting string.
/// The underlying buffer is allocated in thread local storage.
TOOLBOX_API std::string_view dtos(double d) noexcept;

/// Convert double to a fixed format string.
/// Returns the size of the resulting string.
/// The resulting string is not null terminated.
TOOLBOX_API std::size_t dtofixed(char* dst, double d, int prec = 9) noexcept;

/// Convert double to fixed format string.
/// Returns a string_view of the resulting string.
/// The underlying buffer is allocated in thread local storage.
TOOLBOX_API std::string_view dtofixed(double d, int prec = 9) noexcept;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_RYU_HPP
