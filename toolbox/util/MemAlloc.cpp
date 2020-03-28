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

#include "MemAlloc.hpp"

namespace toolbox {
inline namespace util {
using namespace std;

TOOLBOX_WEAK void* alloc(size_t size);
TOOLBOX_WEAK void* alloc(size_t size, align_val_t al);
TOOLBOX_WEAK void dealloc(void* ptr, size_t size) noexcept;
TOOLBOX_WEAK void dealloc(void* ptr, size_t size, align_val_t al) noexcept;

void* alloc(size_t size)
{
    return ::operator new(size);
}

void* alloc(size_t size, align_val_t al)
{
    return ::operator new(size, al);
}

void dealloc(void* ptr, size_t size) noexcept
{
#if __cpp_sized_deallocation
    ::operator delete(ptr, size);
#else
    ::operator delete(ptr);
#endif
}

void dealloc(void* ptr, size_t size, align_val_t al) noexcept
{
#if __cpp_sized_deallocation
    ::operator delete(ptr, size, al);
#else
    ::operator delete(ptr, al);
#endif
}

} // namespace util
} // namespace toolbox
