// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2023 Reactive Markets Limited
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

#include "Frame.hpp"

namespace toolbox {
inline namespace net {
using namespace std;

// Native little endian (least first) and network little endian: no swap required.
static_assert(detail::get_length<std::endian::little>("\002\001", std::endian::little) == 0x0102);
// Native little endian (least first) and network big endian: swap required.
static_assert(detail::get_length<std::endian::little>("\001\002", std::endian::big) == 0x0102);
// Native bid endian and network bid endian: no swap required.
static_assert(detail::get_length<std::endian::big>("\001\002", std::endian::big) == 0x0201);
// Native bid endian and network little endian: swap required.
static_assert(detail::get_length<std::endian::big>("\002\001", std::endian::little) == 0x0201);

} // namespace net
} // namespace toolbox
