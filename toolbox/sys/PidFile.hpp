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

#ifndef TOOLBOX_SYS_PIDFILE_HPP
#define TOOLBOX_SYS_PIDFILE_HPP

#include <toolbox/Config.h>

#include <memory>

struct pidfh;

namespace toolbox {
inline namespace sys {
namespace detail {
struct TOOLBOX_API PidFileDeleter {
    void operator()(pidfh* pfh) const noexcept;
};
} // namespace detail

using PidFile = std::unique_ptr<pidfh, detail::PidFileDeleter>;

/// Create pidfile and obtain lock. An exception is thrown if the pidfile cannot be locked, because a
/// daemon is already running. If the path argument is null, then /var/run/{progname}.pid is used as
/// the pidfile location.
TOOLBOX_API PidFile open_pid_file(const char* path, mode_t mode);

/// Close pidfile without removing it. This function should be used when forking daemon processes.
TOOLBOX_API void close_pid_file(PidFile& pf) noexcept;

/// Write process' PID into pidfile.
TOOLBOX_API void write_pid_file(PidFile& pf);

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_PIDFILE_HPP
