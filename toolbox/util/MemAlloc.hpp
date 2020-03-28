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

#ifndef TOOLBOX_UTIL_MEMALLOC_HPP
#define TOOLBOX_UTIL_MEMALLOC_HPP

#include <toolbox/Config.h>

#include <new>

namespace toolbox {
inline namespace util {

TOOLBOX_API void* alloc(std::size_t size);
TOOLBOX_API void* alloc(std::size_t size, std::align_val_t al);
TOOLBOX_API void dealloc(void* ptr, std::size_t size) noexcept;
TOOLBOX_API void dealloc(void* ptr, std::size_t size, std::align_val_t al) noexcept;

struct MemAlloc {
    static void* operator new(std::size_t size) { return alloc(size); }
    static void* operator new(std::size_t size, std::align_val_t al) { return alloc(size, al); }
    static void operator delete(void* ptr, std::size_t size) noexcept { return dealloc(ptr, size); }
    static void operator delete(void* ptr, std::size_t size, std::align_val_t al) noexcept
    {
        return dealloc(ptr, size, al);
    }

  protected:
    ~MemAlloc() = default;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_MEMALLOC_HPP
