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

#ifndef TOOLBOX_SYS_LIMITS_HPP
#define TOOLBOX_SYS_LIMITS_HPP

#include <toolbox/util/Math.hpp>

namespace toolbox {
inline namespace sys {

// Assumptions:
// sysconf(_SC_LEVEL1_DCACHE_LINESIZE) == 64
// sysconf(_SC_PAGESIZE) == 4096

enum : std::size_t {
    CacheLineBits = 6,
    CacheLineSize = 1 << CacheLineBits,
    PageBits = 12,
    PageSize = 1 << PageBits,
    /// Maximum message size.
    /// A sensible upper-bound for message payloads is 1400 bytes:
    /// - Max Datagram (1472) - Aeron Header (32) = 1440
    /// - Round-down to cache-line boundary: (1440 & ~63) = 1408
    /// - Subtract MPMC queue header: 1408 - 8 = 1400
    MaxMsgSize = 1400
};

constexpr std::size_t ceil_cache_line(std::size_t size) noexcept
{
    return ceil_pow2<CacheLineBits>(size);
}

constexpr std::size_t ceil_page(std::size_t size) noexcept
{
    return ceil_pow2<PageBits>(size);
}

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_LIMITS_HPP
