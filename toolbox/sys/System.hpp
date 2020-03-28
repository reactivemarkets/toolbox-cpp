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

#ifndef TOOLBOX_SYS_SYSTEM_HPP
#define TOOLBOX_SYS_SYSTEM_HPP

#include <toolbox/sys/Error.hpp>

#include <unistd.h>

namespace toolbox {
namespace os {

/// Change working directory.
inline void chdir(const char* path, std::error_code& ec) noexcept
{
    const auto ret = ::chdir(path);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
}

/// Change working directory.
inline void chdir(const char* path)
{
    const auto ret = ::chdir(path);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "chdir"};
    }
}

/// Create a child process.
inline pid_t fork(std::error_code& ec) noexcept
{
    const auto pid = ::fork();
    if (pid < 0) {
        ec = make_sys_error(errno);
    }
    return pid;
}

/// Create a child process.
inline pid_t fork()
{
    const auto pid = ::fork();
    if (pid < 0) {
        throw std::system_error{make_sys_error(errno), "fork"};
    }
    return pid;
}

/// Creates a session and sets the process group ID.
inline pid_t setsid(std::error_code& ec) noexcept
{
    const auto sid = ::setsid();
    if (sid < 0) {
        ec = make_sys_error(errno);
    }
    return sid;
}

/// Creates a session and sets the process group ID.
inline pid_t setsid()
{
    const auto sid = ::setsid();
    if (sid < 0) {
        throw std::system_error{make_sys_error(errno), "setsid"};
    }
    return sid;
}

} // namespace os
inline namespace sys {
} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_SYSTEM_HPP
