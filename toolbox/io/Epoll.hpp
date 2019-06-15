// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
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

#ifndef TOOLBOX_IO_EPOLL_HPP
#define TOOLBOX_IO_EPOLL_HPP

#include <toolbox/io/Handle.hpp>
#include <toolbox/sys/Error.hpp>

#include <sys/epoll.h>

namespace toolbox {
namespace os {

/// Open an epoll file descriptor
inline FileHandle epoll_create(int size, std::error_code& ec) noexcept
{
    const auto ret = ::epoll_create(size);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Open an epoll file descriptor
inline FileHandle epoll_create(int size)
{
    const auto fd = ::epoll_create(size);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "epoll_create"};
    }
    return fd;
}

/// Control interface for an epoll file descriptor.
inline int epoll_ctl(int epfd, int op, int fd, epoll_event event, std::error_code& ec) noexcept
{
    const auto ret = ::epoll_ctl(epfd, op, fd, &event);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Control interface for an epoll file descriptor.
inline void epoll_ctl(int epfd, int op, int fd, epoll_event event)
{
    const auto ret = ::epoll_ctl(epfd, op, fd, &event);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "epoll_ctl"};
    }
}

/// Wait for an I/O event on an epoll file descriptor.
inline int epoll_wait(int epfd, epoll_event* events, int maxevents, int timeout,
                      std::error_code& ec) noexcept
{
    const auto ret = ::epoll_wait(epfd, events, maxevents, timeout);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Wait for an I/O event on an epoll file descriptor.
inline int epoll_wait(int epfd, epoll_event* events, int maxevents, int timeout)
{
    const auto ret = ::epoll_wait(epfd, events, maxevents, timeout);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "epoll_wait"};
    }
    return ret;
}

} // namespace os
inline namespace io {
} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_EPOLL_HPP
