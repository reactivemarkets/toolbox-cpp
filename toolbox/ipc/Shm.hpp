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

#ifndef TOOLBOX_IPC_SHM_HPP
#define TOOLBOX_IPC_SHM_HPP

#include <toolbox/io/Handle.hpp>
#include <toolbox/sys/Error.hpp>

#include <memory>

#include <sys/shm.h>

namespace toolbox {
inline namespace ipc {

struct ShmPolicy {
    using Id = int;
    static constexpr int invalid() noexcept { return -1; }
    static void close(int id) noexcept { ::shmctl(id, IPC_RMID, nullptr); }
};

/// ShmId can be used to ensure that a shared-memory segment is deleted when then handle goes out of
/// scope. This is typically used for managing temporary IPC_PRIVATE segments.
using ShmId = BasicHandle<ShmPolicy>;

template <typename ValueT>
using ShmPtr = std::unique_ptr<ValueT, int (*)(const void*)>;

} // namespace ipc
namespace os {

/// Generates a System V IPC key for a given path and project-id.
inline key_t ftok(const char* path, int proj_id, std::error_code& ec) noexcept
{
    const auto key = ::ftok(path, proj_id);
    if (key < 0) {
        ec = make_sys_error(errno);
    }
    return key;
}

/// Generates a System V IPC key for a given path and project-id.
inline key_t ftok(const char* path, int proj_id)
{
    const auto key = ::ftok(path, proj_id);
    // N.B. Negative values other than -1 are valid keys.
    if (key == -1) {
        throw std::system_error{make_sys_error(errno), "ftok"};
    }
    return key;
}

/// Returns the shared-memory identifier associated with key.
inline int shmget(key_t key, std::size_t size, int shmflg, std::error_code& ec) noexcept
{
    const auto id = ::shmget(key, size, shmflg);
    if (id < 0) {
        ec = make_sys_error(errno);
    }
    return id;
}

/// Returns the shared-memory identifier associated with key.
inline int shmget(key_t key, std::size_t size, int shmflg)
{
    const auto id = ::shmget(key, size, shmflg);
    if (id < 0) {
        throw std::system_error{make_sys_error(errno), "shmget"};
    }
    return id;
}

/// Attach the shared-memory segment associated with \p shmid to the address space of the calling
/// thread.
template <typename ValueT>
ShmPtr<ValueT> shmat(int shmid, const ValueT* shmaddr, int shmflg, std::error_code& ec) noexcept
{
    void* const addr = ::shmat(shmid, shmaddr, shmflg);
    if (reinterpret_cast<long>(addr) == -1) {
        ec = make_sys_error(errno);
        return {nullptr, [](const void*) -> int { return 0; }};
    }
    return {static_cast<ValueT*>(addr), shmdt};
}

/// Attach the shared-memory segment associated with \p shmid to the address space of the calling
/// thread.
template <typename ValueT>
ShmPtr<ValueT> shmat(int shmid, const ValueT* shmaddr, int shmflg)
{
    void* const addr = ::shmat(shmid, shmaddr, shmflg);
    if (reinterpret_cast<long>(addr) == -1) {
        throw std::system_error{make_sys_error(errno), "shmat"};
    }
    return {static_cast<ValueT*>(addr), shmdt};
}

} // namespace os
} // namespace toolbox

#endif // TOOLBOX_IPC_SHM_HPP
