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

#ifndef TOOLBOX_UTIL_BITS_HPP
#define TOOLBOX_UTIL_BITS_HPP

namespace toolbox {
inline namespace util {

/// Returns the number of leading 0-bits, starting at the most significant bit position. The result
/// is undefined for zero values.
///
/// \tparam ValueT Integer type.
/// \param i Integer value.
/// \return the number of leading 0-bits.
template <typename IntegerT>
constexpr auto clz(IntegerT i) noexcept
{
    if constexpr (sizeof(i) <= sizeof(int)) {
        return __builtin_clz(i);
    } else if constexpr (sizeof(i) <= sizeof(long)) {
        return __builtin_clzl(i);
    } else {
        return __builtin_clzll(i);
    }
}

static_assert(clz(~0) == 0);
static_assert(clz(1) == 31);

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_BITS_HPP
