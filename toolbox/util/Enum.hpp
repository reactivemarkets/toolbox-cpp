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

#ifndef TOOLBOX_UTIL_ENUM_HPP
#define TOOLBOX_UTIL_ENUM_HPP

#include <iosfwd>
#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename EnumT, typename = std::enable_if_t<std::is_enum_v<EnumT>>>
constexpr EnumT box(typename std::underlying_type_t<EnumT> val) noexcept
{
    return static_cast<EnumT>(val);
}

template <typename EnumT, typename = std::enable_if_t<std::is_enum_v<EnumT>>>
constexpr typename std::underlying_type_t<EnumT> unbox(EnumT val) noexcept
{
    return static_cast<std::underlying_type_t<EnumT>>(val);
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ENUM_HPP
