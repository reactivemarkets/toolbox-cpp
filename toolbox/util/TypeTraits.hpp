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

#ifndef TOOLBOX_UTIL_TYPETRAITS_HPP
#define TOOLBOX_UTIL_TYPETRAITS_HPP

#include <toolbox/util/Utility.hpp>

#include <string>

namespace toolbox {
inline namespace util {

template <typename ValueT, typename EnableT = void>
struct TypeTraits;

template <typename ValueT>
struct TypeTraits<ValueT, std::enable_if_t<std::is_integral_v<ValueT>>> {
    static constexpr auto from_string(std::string_view sv) noexcept { return ston<ValueT>(sv); }
    static constexpr auto from_string(const std::string& s) noexcept
    {
        return from_string(std::string_view{s});
    }
};

template <>
struct TypeTraits<bool> {
    static auto from_string(std::string_view sv) noexcept { return stob(sv); }
    static auto from_string(const std::string& s) noexcept
    {
        return from_string(std::string_view{s});
    }
};

template <>
struct TypeTraits<double> {
    static auto from_string(std::string_view sv) noexcept { return stod(sv); }
    static auto from_string(const std::string& s) noexcept
    {
        return from_string(std::string_view{s});
    }
};

template <>
struct TypeTraits<std::string_view> {
    static constexpr auto from_string(std::string_view sv) noexcept { return sv; }
    /// Disable conversion from std::string to std::string_view due to possible danger of dangling
    /// reference to temporary.
    static std::string_view from_string(const std::string& s) = delete;
};

template <>
struct TypeTraits<std::string> {
    static std::string from_string(std::string_view sv) noexcept { return {sv.data(), sv.size()}; }
    static std::string from_string(const std::string& s) noexcept { return s; }
};

template <typename TypeT>
struct is_string : std::is_same<char*, std::remove_cv_t<typename std::decay_t<TypeT>>>::type {
};
template <>
struct is_string<std::string> : std::true_type {
};
template <>
struct is_string<std::string_view> : std::true_type {
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TYPETRAITS_HPP
