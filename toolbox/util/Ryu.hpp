// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
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

#include <cstddef> // size_t

namespace toolbox {
inline namespace util {

/// Maximum buffer space require for Ryu formatted doubles. This upper-bound is chosen based on the
/// internal buffer allocation within the d2s function. See d2s.c for details.
constexpr std::size_t MaxRyuBuf{24};

TOOLBOX_API std::size_t d2s_buffered_n(double f, char* result) noexcept;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_RYU_HPP
