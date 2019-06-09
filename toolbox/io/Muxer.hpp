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

#ifndef TOOLBOX_IO_MUXER_HPP
#define TOOLBOX_IO_MUXER_HPP

#include <toolbox/io/Epoll.hpp>
#include <toolbox/io/Event.hpp>
#include <toolbox/io/TimerFd.hpp>

#include <toolbox/Config.h>

#include <unistd.h>

namespace toolbox {
inline namespace io {

class EpollMuxer {
  public:
    using Event = epoll_event;

    static constexpr int fd(const Event& ev) noexcept
    {
        return static_cast<int>(ev.data.u64 & 0xffffffff);
    }
    static constexpr int sid(const Event& ev) noexcept
    {
        return static_cast<int>(ev.data.u64 >> 32);
    }
    static constexpr unsigned events(const Event& ev) noexcept
    {
        unsigned n{};
        if (ev.events & EPOLLIN) {
            n |= EventIn;
        }
        if (ev.events & EPOLLPRI) {
            n |= EventPri;
        }
        if (ev.events & EPOLLOUT) {
            n |= EventOut;
        }
        if (ev.events & EPOLLERR) {
            n |= EventErr;
        }
        if (ev.events & EPOLLHUP) {
            n |= EventHup;
        }
        if (ev.events & EPOLLET) {
            n |= EventEt;
        }
        return n;
    }

    explicit EpollMuxer(std::size_t size_hint)
    : mux_{os::epoll_create(size_hint)}
    , tfd_{TFD_NONBLOCK}
    {
        subscribe(tfd_.fd(), 0, EventIn);
    }
    ~EpollMuxer() { unsubscribe(tfd_.fd()); }

    // Copy.
    EpollMuxer(const EpollMuxer&) = delete;
    EpollMuxer& operator=(const EpollMuxer&) = delete;

    // Move.
    EpollMuxer(EpollMuxer&&) = default;
    EpollMuxer& operator=(EpollMuxer&&) = default;

    void swap(EpollMuxer& rhs) noexcept { std::swap(mux_, rhs.mux_); }
    /// Returns the number of file descriptors that are ready.
    int wait(Event buf[], std::size_t size, std::error_code& ec)
    {
        // A zero timeout will disarm the timer.
        tfd_.set_time(0, MonoTime{}, ec);
        if (ec) {
            return 0;
        }
        return os::epoll_wait(*mux_, buf, size, -1, ec);
    }
    /// Returns the number of file descriptors that are ready, or zero if no file descriptor became
    /// ready during before the operation timed-out.
    int wait(Event buf[], std::size_t size, MonoTime timeout, std::error_code& ec)
    {
        // A zero timeout will disarm the timer.
        tfd_.set_time(0, timeout, ec);
        if (ec) {
            return 0;
        }
        // Do not block if timer is zero.
        return os::epoll_wait(*mux_, buf, size, is_zero(timeout) ? 0 : -1, ec);
    }
    void subscribe(int fd, int sid, unsigned events)
    {
        Event ev;
        set_events(ev, fd, sid, events);
        os::epoll_ctl(*mux_, EPOLL_CTL_ADD, fd, ev);
    }
    void unsubscribe(int fd) noexcept
    {
        // In kernel versions before 2.6.9, the EPOLL_CTL_DEL operation required a non-null pointer
        // in event, even though this argument is ignored.
        Event ev{};
        std::error_code ec;
        os::epoll_ctl(*mux_, EPOLL_CTL_DEL, fd, ev, ec);
    }
    void set_events(int fd, int sid, unsigned events, std::error_code& ec) noexcept
    {
        Event ev;
        set_events(ev, fd, sid, events);
        os::epoll_ctl(*mux_, EPOLL_CTL_MOD, fd, ev, ec);
    }
    void set_events(int fd, int sid, unsigned events)
    {
        Event ev;
        set_events(ev, fd, sid, events);
        os::epoll_ctl(*mux_, EPOLL_CTL_MOD, fd, ev);
    }

  private:
    static void set_events(Event& ev, int fd, int sid, unsigned events) noexcept
    {
        unsigned n{};
        if (events & EventIn) {
            n |= EPOLLIN;
        }
        if (events & EventPri) {
            n |= EPOLLPRI;
        }
        if (events & EventOut) {
            n |= EPOLLOUT;
        }
        if (events & EventErr) {
            n |= EPOLLERR;
        }
        if (events & EventHup) {
            n |= EPOLLHUP;
        }
        if (events & EventEt) {
            n |= EPOLLET;
        }
        ev.events = n;
        ev.data.u64 = static_cast<std::uint64_t>(sid) << 32 | fd;
    }
    FileHandle mux_;
    TimerFd<MonoClock> tfd_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_MUXER_HPP
