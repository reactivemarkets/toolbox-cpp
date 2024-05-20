// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_ALLOCATOR_HPP
#define TOOLBOX_UTIL_ALLOCATOR_HPP

#include <toolbox/Config.h>

#include <cstddef>

namespace toolbox {

TOOLBOX_API void* allocate(std::size_t size);
TOOLBOX_API void deallocate(void* ptr, std::size_t size) noexcept;

inline namespace util {

struct Allocator {
    static void* operator new(std::size_t size) { return allocate(size); }
    static void operator delete(void* ptr, std::size_t size) noexcept
    {
        return deallocate(ptr, size);
    }

  protected:
    ~Allocator() = default;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ALLOCATOR_HPP
