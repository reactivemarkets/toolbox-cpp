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

#include "Daemon.hpp"

#include "System.hpp"

#include <fcntl.h> // open()

namespace toolbox {
inline namespace sys {
using namespace std;

void close_all() noexcept
{
    // Impose upper bound because Docker containers may have a very large default.
    const int fds{min(getdtablesize(), 1024)};
    for (int fd{STDERR_FILENO + 1}; fd < fds; ++fd) {
        close(fd);
    }
}

void daemon()
{
    pid_t pid{os::fork()};
    if (pid != 0) {
        // Exit parent process using system version of exit() to avoid flushing standard streams.
        // FIXME: use quick_exit() when available on OSX.
        _exit(0);
    }

    // Detach from controlling terminal by making process a session leader.
    os::setsid();

    // Forking again ensures that the daemon process is not a session leader, and therefore cannot
    // regain access to a controlling terminal.
    pid = os::fork();
    if (pid != 0) {
        // FIXME: use quick_exit() when available on OSX.
        _exit(0);
    }

    // Re-open standard input.
    close(STDIN_FILENO);
    const auto fd = ::open("/dev/null", O_RDONLY);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "open"};
    }
}

} // namespace sys
} // namespace toolbox
