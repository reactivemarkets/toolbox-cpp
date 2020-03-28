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

#ifndef TOOLBOX_UTIL_PATH_HPP
#define TOOLBOX_UTIL_PATH_HPP

#include <toolbox/util/TypeTraits.hpp>

#include <experimental/filesystem>

namespace toolbox {
inline namespace util {
using Path = std::experimental::filesystem::path;

template <>
struct TypeTraits<Path> {
    static auto from_string(std::string_view sv) noexcept { return Path{sv}; }
    static auto from_string(const std::string& s) noexcept { return Path{s}; }
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_PATH_HPP
