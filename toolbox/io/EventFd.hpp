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

#ifndef TOOLBOX_IO_EVENTFD_HPP
#define TOOLBOX_IO_EVENTFD_HPP

#include <toolbox/io/File.hpp>

#include <sys/eventfd.h>

namespace toolbox {
namespace os {

/// Create a file descriptor for event notification.
inline FileHandle eventfd(unsigned intval, int flags, std::error_code& ec) noexcept
{
    const auto fd = ::eventfd(intval, flags);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Create a file descriptor for event notification.
inline FileHandle eventfd(unsigned intval, int flags)
{
    const auto fd = ::eventfd(intval, flags);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "eventfd"};
    }
    return fd;
}

} // namespace os
inline namespace io {

class EventFd {
  public:
    EventFd(unsigned intval, int flags)
    : fh_{os::eventfd(intval, flags)}
    {
    }
    ~EventFd() = default;

    // Copy.
    EventFd(const EventFd&) = delete;
    EventFd& operator=(const EventFd&) = delete;

    // Move.
    EventFd(EventFd&&) = default;
    EventFd& operator=(EventFd&&) = default;

    int fd() const noexcept { return fh_.get(); }
    std::int64_t read()
    {
        union {
            char buf[sizeof(std::int64_t)];
            std::int64_t val;
        } u;
        os::read(*fh_, u.buf, sizeof(u.buf));
        return u.val;
    }
    void write(std::int64_t val, std::error_code& ec) noexcept
    {
        // Adds the 8-byte integer value supplied in its buffer to the counter.
        union {
            char buf[sizeof(std::int64_t)];
            std::int64_t val;
        } u;
        u.val = val;
        os::write(*fh_, u.buf, sizeof(u.buf), ec);
    }
    void write(std::int64_t val)
    {
        // Adds the 8-byte integer value supplied in its buffer to the counter.
        union {
            char buf[sizeof(std::int64_t)];
            std::int64_t val;
        } u;
        u.val = val;
        os::write(*fh_, u.buf, sizeof(u.buf));
    }

  private:
    FileHandle fh_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_EVENTFD_HPP
