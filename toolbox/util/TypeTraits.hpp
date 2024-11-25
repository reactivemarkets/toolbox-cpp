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

#ifndef TOOLBOX_UTIL_TYPETRAITS_HPP
#define TOOLBOX_UTIL_TYPETRAITS_HPP

#include <toolbox/util/Utility.hpp>

#include <string>
#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename ValueT>
struct TypeTraits {
    template <typename StringT>
    static constexpr auto from_string(StringT&& s) noexcept(noexcept(ValueT{s}))
    {
        return ValueT{s};
    }
};

template <typename ValueT>
    requires std::integral<ValueT>
struct TypeTraits<ValueT> {
    static constexpr auto from_string(std::string_view sv) noexcept { return ston<ValueT>(sv); }
};

template <>
struct TypeTraits<bool> {
    static auto from_string(std::string_view sv) noexcept { return stob(sv); }
};

template <>
struct TypeTraits<double> {
    static auto from_string(std::string_view sv) noexcept { return stod(sv); }
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
    static std::string from_string(std::string_view sv) { return {sv.data(), sv.size()}; }
    static std::string from_string(const std::string& s) { return s; }
};

template <typename T, template <typename...> class Tpl>
struct is_instantiation_of_helper : std::false_type {};

template <typename... Ts, template <typename...> class Tpl>
struct is_instantiation_of_helper<Tpl<Ts...>, Tpl> : std::true_type {};

template <typename T, template <typename...> class Tpl>
struct is_instantiation_of : is_instantiation_of_helper<std::remove_cv_t<T>, Tpl> {};

template <typename T, template <typename...> class Tpl>
inline constexpr bool is_instantiation_of_v = is_instantiation_of<T, Tpl>::value;

template <typename T>
using is_string = is_instantiation_of<T, std::basic_string>;

template <typename T>
inline constexpr bool is_string_v = is_string<T>::value;

template <typename T>
using is_string_view = is_instantiation_of<T, std::basic_string_view>;

template <typename T>
inline constexpr bool is_string_view_v = is_string_view<T>::value;

template <typename T>
struct is_decay_to_cstring_helper : std::false_type {};

template <typename T>
struct is_decay_to_cstring_helper<T*> : std::is_same<std::remove_cv_t<T>, char> {};

template <typename T>
struct is_decay_to_cstring : is_decay_to_cstring_helper<std::remove_cv_t<std::decay_t<T>>> {};

template <typename T>
inline constexpr bool is_decay_to_cstring_v = is_decay_to_cstring<T>::value;

template <typename T>
struct is_string_type : std::integral_constant<bool, is_string_v<T> ||
                                                     is_string_view_v<T> ||
                                                     is_decay_to_cstring_v<T>> {};

template <typename T>
inline constexpr bool is_string_type_v = is_string_type<T>::value;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TYPETRAITS_HPP
