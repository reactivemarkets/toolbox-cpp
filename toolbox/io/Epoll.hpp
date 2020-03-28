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

#ifndef TOOLBOX_IO_EPOLL_HPP
#define TOOLBOX_IO_EPOLL_HPP

#include <toolbox/io/Event.hpp>
#include <toolbox/io/Handle.hpp>
#include <toolbox/io/TimerFd.hpp>
#include <toolbox/sys/Error.hpp>

#include <sys/epoll.h>

namespace toolbox {
namespace os {

/// Open a new epoll instance. The size argument is ignored in kernels >= 2.6.8, but must be greater
/// than zero.
inline FileHandle epoll_create(int size, std::error_code& ec) noexcept
{
    const auto ret = ::epoll_create(size);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Open a new epoll instance. The size argument is ignored in kernels >= 2.6.8, but must be greater
/// than zero.
inline FileHandle epoll_create(int size)
{
    const auto fd = ::epoll_create(size);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "epoll_create"};
    }
    return fd;
}

/// Open a new epoll instance. If flags is zero, then epoll_create1() is the same as epoll_create().
inline FileHandle epoll_create1(int flags, std::error_code& ec) noexcept
{
    const auto ret = ::epoll_create1(flags);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// Open a new epoll instance. If flags is zero, then epoll_create1() is the same as epoll_create().
inline FileHandle epoll_create1(int flags)
{
    const auto fd = ::epoll_create1(flags);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "epoll_create1"};
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

enum : unsigned {
    /// The associated file is available for read(2) operations.
    EpollIn = EPOLLIN,

    /// The associated file is available for write(2) operations.
    EpollOut = EPOLLOUT,

    /// Stream socket peer closed connection, or shut down writing half of connection. (This flag is
    /// especially useful for writing simple code to detect peer shutdown when using Edge-Triggered
    /// monitoring.)
    EpollRdHup = EPOLLRDHUP,

    /// There is an exceptional condition on the file descriptor. See the discussion of POLLPRI in
    /// poll(2).
    EpollPri = EPOLLPRI,

    /// Error condition happened on the associated file descriptor. This event is also reported for
    /// the write end of a pipe when the read end has been closed. epoll_wait(2) will always report
    /// for this event; it is not necessary to set it in events.
    EpollErr = EPOLLERR,

    /// Hang up happened on the associated file descriptor. epoll_wait(2) will always wait for this
    /// event; it is not necessary to set it in events.
    ///
    /// Note that when reading from a channel such as a pipe or a stream socket, this event merely
    /// indicates that the peer closed its end of the channel. Subsequent reads from the channel
    /// will return 0 (end of file) only after all outstanding data in the channel has been
    /// consumed.
    EpollHup = EPOLLHUP,

    /// Sets the Edge-Triggered behavior for the associated file descriptor. The default behavior
    /// for epoll is Level-Triggered. See epoll(7) for more detailed information about Edge and
    /// Level-Triggered event distribution architectures.
    EpollEt = EPOLLET,

    /// Sets the one-shot behavior for the associated file descriptor. This means that after an
    /// event is pulled out with epoll_wait(2) the associated file descriptor is internally disabled
    /// and no other events will be reported by the epoll interface. The user must call epoll_ctl()
    /// with EPOLL_CTL_MOD to rearm the file descriptor with a new event mask.
    EpollOneShot = EPOLLONESHOT,
};

using EpollEvent = epoll_event;

class Epoll {
  public:
    using Event = EpollEvent;

    static constexpr int fd(const Event& ev) noexcept
    {
        return static_cast<int>(ev.data.u64 & 0xffffffff);
    }
    static constexpr int sid(const Event& ev) noexcept
    {
        return static_cast<int>(ev.data.u64 >> 32);
    }
    explicit Epoll(int flags = 0)
    : epfd_{os::epoll_create1(flags)}
    , tfd_{TFD_NONBLOCK}
    {
        add(tfd_.fd(), 0, EpollIn);
    }
    ~Epoll() { del(tfd_.fd()); }

    // Copy.
    Epoll(const Epoll&) = delete;
    Epoll& operator=(const Epoll&) = delete;

    // Move.
    Epoll(Epoll&&) = default;
    Epoll& operator=(Epoll&&) = default;

    /// Returns the timer file descriptor.
    /// Exposing the timer file descriptor allows callers to check if one of the signalled events
    /// was trigger by the timer.
    int timer_fd() const noexcept { return tfd_.fd(); }

    void swap(Epoll& rhs) noexcept { std::swap(epfd_, rhs.epfd_); }

    /// Returns the number of file descriptors that are ready.
    int wait(Event buf[], std::size_t size, std::error_code& ec) noexcept
    {
        MonoTime timeout{};
        // Only set the timer if it has changed.
        if (timeout != timeout_) {
            // A zero timeout will disarm the timer.
            tfd_.set_time(0, timeout, ec);
            if (ec) {
                return 0;
            }
            timeout_ = timeout;
        }
        return os::epoll_wait(*epfd_, buf, size, -1, ec);
    }
    /// Returns the number of file descriptors that are ready, or zero if no file descriptor became
    /// ready during before the operation timed-out.
    /// The number of file descriptors returnes may include the timer file descriptor,
    /// so callers must check for the presence of this descriptor.
    int wait(Event buf[], std::size_t size, MonoTime timeout, std::error_code& ec) noexcept
    {
        // Only set the timer if it has changed.
        if (timeout != timeout_) {
            // A zero timeout will disarm the timer.
            tfd_.set_time(0, timeout, ec);
            if (ec) {
                return 0;
            }
            timeout_ = timeout;
        }
        // Do not block if timer is zero.
        return os::epoll_wait(*epfd_, buf, size, is_zero(timeout) ? 0 : -1, ec);
    }
    void add(int fd, int sid, unsigned events)
    {
        Event ev;
        mod(ev, fd, sid, events);
        os::epoll_ctl(*epfd_, EPOLL_CTL_ADD, fd, ev);
    }
    void del(int fd) noexcept
    {
        // In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-null pointer
        // in event, even though this argument is ignored.
        Event ev{};
        std::error_code ec;
        os::epoll_ctl(*epfd_, EPOLL_CTL_DEL, fd, ev, ec);
    }
    void mod(int fd, int sid, unsigned events, std::error_code& ec) noexcept
    {
        Event ev;
        mod(ev, fd, sid, events);
        os::epoll_ctl(*epfd_, EPOLL_CTL_MOD, fd, ev, ec);
    }
    void mod(int fd, int sid, unsigned events)
    {
        Event ev;
        mod(ev, fd, sid, events);
        os::epoll_ctl(*epfd_, EPOLL_CTL_MOD, fd, ev);
    }

  private:
    static void mod(Event& ev, int fd, int sid, unsigned events) noexcept
    {
        ev.events = events;
        ev.data.u64 = static_cast<std::uint64_t>(sid) << 32 | fd;
    }
    FileHandle epfd_;
    TimerFd<MonoClock> tfd_;
    MonoTime timeout_{};
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_EPOLL_HPP
