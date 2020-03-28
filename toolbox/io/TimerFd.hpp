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

#ifndef TOOLBOX_IO_TIMERFD_HPP
#define TOOLBOX_IO_TIMERFD_HPP

#include <toolbox/io/File.hpp>
#include <toolbox/sys/Time.hpp>

#include <sys/timerfd.h>

namespace toolbox {
namespace os {

/// Create a file descriptor for timer notification.
inline FileHandle timerfd_create(int clock_id, int flags, std::error_code& ec) noexcept
{
    const auto fd = ::timerfd_create(clock_id, flags);
    if (fd < 0) {
        ec = make_sys_error(errno);
    }
    return fd;
}

/// Create a file descriptor for timer notification.
inline FileHandle timerfd_create(int clock_id, int flags)
{
    const auto fd = ::timerfd_create(clock_id, flags);
    if (fd < 0) {
        throw std::system_error{make_sys_error(errno), "timerfd_create"};
    }
    return fd;
}

/// Arm or disarm timer.
inline void timerfd_settime(int fd, int flags, const itimerspec& new_value, itimerspec& old_value,
                            std::error_code& ec) noexcept
{
    const auto ret = ::timerfd_settime(fd, flags, &new_value, &old_value);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
}

/// Arm or disarm timer.
inline void timerfd_settime(int fd, int flags, const itimerspec& new_value, itimerspec& old_value)
{
    const auto ret = ::timerfd_settime(fd, flags, &new_value, &old_value);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "timerfd_settime"};
    }
}

/// Arm or disarm timer.
inline void timerfd_settime(int fd, int flags, const itimerspec& new_value,
                            std::error_code& ec) noexcept
{
    const auto ret = ::timerfd_settime(fd, flags, &new_value, nullptr);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
}

/// Arm or disarm timer.
inline void timerfd_settime(int fd, int flags, const itimerspec& new_value)
{
    const auto ret = ::timerfd_settime(fd, flags, &new_value, nullptr);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "timerfd_settime"};
    }
}

/// Arm or disarm timer.
template <typename ClockT>
inline void timerfd_settime(int fd, int flags, std::chrono::time_point<ClockT, Duration> expiry,
                            Duration interval, std::error_code& ec) noexcept
{
    return timerfd_settime(fd, flags | TFD_TIMER_ABSTIME,
                           {to_timespec(interval), to_timespec(expiry)}, ec);
}

/// Arm or disarm timer.
template <typename ClockT>
inline void timerfd_settime(int fd, int flags, std::chrono::time_point<ClockT, Duration> expiry,
                            Duration interval)
{
    return timerfd_settime(fd, flags | TFD_TIMER_ABSTIME,
                           {to_timespec(interval), to_timespec(expiry)});
}

/// Arm or disarm timer.
template <typename ClockT>
inline void timerfd_settime(int fd, int flags, std::chrono::time_point<ClockT, Duration> expiry,
                            std::error_code& ec) noexcept
{
    return timerfd_settime(fd, flags | TFD_TIMER_ABSTIME, {{}, to_timespec(expiry)}, ec);
}

/// Arm or disarm timer.
template <typename ClockT>
inline void timerfd_settime(int fd, int flags, std::chrono::time_point<ClockT, Duration> expiry)
{
    return timerfd_settime(fd, flags | TFD_TIMER_ABSTIME, {{}, to_timespec(expiry)});
}

} // namespace os
inline namespace io {

template <typename ClockT>
class TimerFd {
  public:
    using Clock = ClockT;
    using TimePoint = std::chrono::time_point<ClockT, Duration>;

    explicit TimerFd(int flags)
    : fh_{os::timerfd_create(Clock::Id, flags)}
    {
    }
    ~TimerFd() = default;

    // Copy.
    TimerFd(const TimerFd&) = delete;
    TimerFd& operator=(const TimerFd&) = delete;

    // Move.
    TimerFd(TimerFd&&) = default;
    TimerFd& operator=(TimerFd&&) = default;

    int fd() const noexcept { return fh_.get(); }

    void set_time(int flags, TimePoint expiry, Duration interval, std::error_code& ec) noexcept
    {
        return os::timerfd_settime(*fh_, flags, expiry, interval, ec);
    }
    void set_time(int flags, TimePoint expiry, Duration interval)
    {
        return os::timerfd_settime(*fh_, flags, expiry, interval);
    }
    void set_time(int flags, TimePoint expiry, std::error_code& ec) noexcept
    {
        return os::timerfd_settime(*fh_, flags, expiry, ec);
    }
    void set_time(int flags, TimePoint expiry) { return os::timerfd_settime(*fh_, flags, expiry); }

  private:
    FileHandle fh_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_TIMERFD_HPP
