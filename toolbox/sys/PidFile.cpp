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

#include "PidFile.hpp"

#include "Error.hpp"

#include <toolbox/util/String.hpp>

#include <toolbox/contrib/flopen.c>
#include <toolbox/contrib/pidfile.c>

// Note that it is safe to pass null to the pidfile_write(), pidfile_remove() and pidfile_close()
// functions.

namespace toolbox {
inline namespace sys {

void detail::PidFileDeleter::operator()(pidfh* pfh) const noexcept
{
    pidfile_remove(pfh);
}

PidFile open_pid_file(const char* path, mode_t mode)
{
    pid_t pid{};
    PidFile pf{pidfile_open(path, mode, &pid)};
    if (!pf) {
        if (errno == EEXIST) {
            throw std::runtime_error{"daemon already running, pid: "s + to_string(pid)};
        }
        throw std::system_error{make_sys_error(errno), "pidfile_open"};
    }
    return pf;
}

void close_pid_file(PidFile& pf) noexcept
{
    if (pf) {
        pidfile_close(pf.release());
    }
}

void write_pid_file(PidFile& pf)
{
    if (pf && pidfile_write(pf.get()) < 0) {
        throw std::system_error{make_sys_error(errno), "pidfile_write"};
    }
}

} // namespace sys
} // namespace toolbox
