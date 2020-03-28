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

#ifndef TOOLBOX_IPC_MMAP_HPP
#define TOOLBOX_IPC_MMAP_HPP

#include <toolbox/sys/Error.hpp>

#include <toolbox/Config.h>

#include <memory>
#include <system_error>

#include <sys/mman.h>

namespace toolbox {
inline namespace ipc {

/// Memory-mapped addressed may be zero (in rare situations), but can never be MAP_FAILED.
class MmapPointer {
  public:
    MmapPointer(std::nullptr_t = nullptr) noexcept {}
    MmapPointer(void* ptr, std::size_t size) noexcept
    : ptr_{ptr}
    , size_{size}
    {
    }
    void* get() const noexcept { return ptr_; }
    void* data() const noexcept { return ptr_; }
    std::size_t size() const noexcept { return size_; }
    explicit operator bool() const noexcept { return ptr_ != MAP_FAILED; }

  private:
    void* ptr_{MAP_FAILED};
    std::size_t size_{0};
};

inline bool operator==(MmapPointer lhs, MmapPointer rhs)
{
    return lhs.get() == rhs.get() && lhs.size() == rhs.size();
}

inline bool operator!=(MmapPointer lhs, MmapPointer rhs)
{
    return !(lhs == rhs);
}

struct MmapDeleter {
    using pointer = MmapPointer;
    void operator()(MmapPointer p) const noexcept
    {
        if (p) {
            munmap(p.get(), p.size());
        }
    }
};

using Mmap = std::unique_ptr<MmapPointer, MmapDeleter>;

} // namespace ipc
namespace os {

/// Map files or devices into memory.
inline Mmap mmap(void* addr, size_t len, int prot, int flags, int fd, off_t off,
                 std::error_code& ec) noexcept
{
    const MmapPointer p{::mmap(addr, len, prot, flags, fd, off), len};
    if (!p) {
        ec = make_sys_error(errno);
    }
    return Mmap{p};
}

inline Mmap mmap(void* addr, size_t len, int prot, int flags, int fd, off_t off)
{
    const MmapPointer p{::mmap(addr, len, prot, flags, fd, off), len};
    if (!p) {
        throw std::system_error{make_sys_error(errno), "mmap"};
    }
    return Mmap{p};
}

} // namespace os
} // namespace toolbox

#endif // TOOLBOX_IPC_MMAP_HPP
