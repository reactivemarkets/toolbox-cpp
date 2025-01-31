// The Reactive C++ Toolbox.
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

#ifndef TOOLBOX_UTIL_CONCEPTS_HPP
#define TOOLBOX_UTIL_CONCEPTS_HPP

#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename T>
concept Arithmetic = std::is_arithmetic_v<T>;

template <typename T>
concept Enum = std::is_enum_v<T>;

// N.B. The concept doesn't check for operator<< overloads on the streamable object.
// Checking for operator<< overloads would've been nice, but it's not possible/difficult due
// to recursions. For example, consider this constraint "os << int/float/etc" -- to check this
// constraint the compiler has to check if there is a valid (unambiguous) operator<< overload.
// However, some generic operator<< overloads for custom types will make use of this concept to
// accept any streamable type -- this is an infinite recursion because the compiler has to then
// check are the constraints satisfied on that overload, which means checking "os << int/float/etc"
// constraint again, checking all operator<< overloads again, etc repeating in a cycle.
template <typename T>
concept Streamable = requires (T& os) {
    os.put(std::declval<char>());
    os.write(std::declval<const char*>(), std::declval<std::size_t>());
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_CONCEPTS_HPP
