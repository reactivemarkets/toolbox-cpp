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

#ifndef TOOLBOX_SYS_TIME_HPP
#define TOOLBOX_SYS_TIME_HPP

#include <toolbox/util/TypeTraits.hpp>

#include <boost/io/ios_state.hpp>

#include <chrono>
#include <iomanip>
#include <iosfwd>
#include <optional>

#include <sys/time.h>

namespace toolbox {
inline namespace sys {
using namespace std::literals::chrono_literals;
using namespace std::literals::string_view_literals;

using Seconds = std::chrono::seconds;
using Millis = std::chrono::milliseconds;
using Micros = std::chrono::microseconds;
using Nanos = std::chrono::nanoseconds;

using NanoTime = Nanos;
using Duration = Nanos;

TOOLBOX_API NanoTime get_time(clockid_t clock_id) noexcept;

struct MonoClock {
    using duration = Duration;
    using period = Duration::period;
    using rep = Duration::rep;
    using time_point = std::chrono::time_point<MonoClock, Duration>;

    static constexpr int Id{CLOCK_MONOTONIC};
    static constexpr bool is_steady{true};

    static constexpr time_point max() noexcept
    {
        using namespace std::chrono;
        using FromPoint = std::chrono::time_point<MonoClock, seconds>;
        constexpr seconds secs{std::numeric_limits<int>::max()};
        return time_point_cast<Duration>(FromPoint{secs});
    }

    static time_point now() noexcept { return time_point{get_time(Id)}; }

    static constexpr std::time_t to_time_t(const time_point& tp) noexcept
    {
        using namespace std::chrono;
        return duration_cast<seconds>(tp.time_since_epoch()).count();
    }

    static constexpr time_point from_time_t(std::time_t t) noexcept
    {
        using namespace std::chrono;
        using FromPoint = std::chrono::time_point<MonoClock, seconds>;
        return time_point_cast<Duration>(FromPoint{seconds{t}});
    }
};

struct WallClock {
    using duration = Duration;
    using period = Duration::period;
    using rep = Duration::rep;
    using time_point = std::chrono::time_point<WallClock, Duration>;

    static constexpr int Id{CLOCK_REALTIME};
    static constexpr bool is_steady{false};

    static constexpr time_point max() noexcept
    {
        using namespace std::chrono;
        using FromPoint = std::chrono::time_point<WallClock, seconds>;
        constexpr seconds secs{std::numeric_limits<int>::max()};
        return time_point_cast<Duration>(FromPoint{secs});
    }

    static time_point now() noexcept { return time_point{get_time(Id)}; }

    static constexpr std::time_t to_time_t(const time_point& tp) noexcept
    {
        using namespace std::chrono;
        return duration_cast<seconds>(tp.time_since_epoch()).count();
    }

    static constexpr time_point from_time_t(std::time_t t) noexcept
    {
        using namespace std::chrono;
        using FromPoint = std::chrono::time_point<WallClock, seconds>;
        return time_point_cast<Duration>(FromPoint{seconds{t}});
    }
};

using MonoTime = MonoClock::time_point;
using WallTime = WallClock::time_point;

TOOLBOX_API std::ostream& operator<<(std::ostream& os, MonoTime t);
TOOLBOX_API std::ostream& operator<<(std::ostream& os, WallTime t);

/// The "cycle-time" represents the start of a processing cycle. This could be, for example, when a
/// thread wakes from a call to epoll_wait().
///
/// Application cycles often require both wall-clock and monotonic time. (Monotonic is primarily
/// used for interval timers.) Rather than pass wall-clock _and_ monotonic time as a parameter down
/// through the function call-tree, these times are cached in a thread-local object.
///
/// The CyclTime tag type (empty class) is designed to make the thread-local programming contract
/// more explicit, without consuming extra registers required for parameter passing - assuming that
/// the optimiser will optimise-away empty tag parameters.
class TOOLBOX_API CyclTime {
  public:
    static CyclTime current() noexcept { return {}; }
    static CyclTime now() noexcept
    {
        time_ = Time::now();
        return {}; // Empty tag.
    }
    /// This overload allows users to override wall-clock time.
    static CyclTime now(WallTime wall_time) noexcept
    {
        time_ = Time::now(wall_time);
        return {}; // Empty tag.
    }
    MonoTime mono_time() const noexcept { return time_.mono_time; }
    WallTime wall_time() const noexcept { return time_.wall_time; }
    void set_wall_time(WallTime wall_time) noexcept { time_.wall_time = wall_time; }

  private:
    CyclTime() = default;
    struct Time {
        static Time now() noexcept { return {MonoClock::now(), WallClock::now()}; };
        static Time now(WallTime wall_time) noexcept { return {MonoClock::now(), wall_time}; };
        MonoTime mono_time{};
        WallTime wall_time{};
    };
    static thread_local Time time_;
};

template <typename RepT, typename PeriodT>
constexpr bool is_zero(std::chrono::duration<RepT, PeriodT> d) noexcept
{
    return d == decltype(d){};
}

template <typename ClockT>
constexpr bool is_zero(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    return t == decltype(t){};
}

template <typename ClockT, typename DurationT>
constexpr DurationT time_since_epoch(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    const Duration d{t.time_since_epoch()};
    return duration_cast<DurationT>(d);
}

template <typename ClockT>
constexpr std::int64_t ms_since_epoch(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    return time_since_epoch<ClockT, milliseconds>(t).count();
}

template <typename ClockT>
constexpr std::int64_t us_since_epoch(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    return time_since_epoch<ClockT, microseconds>(t).count();
}

template <typename ClockT>
constexpr std::int64_t ns_since_epoch(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    const nanoseconds ns{t.time_since_epoch()};
    return ns.count();
}

template <typename ClockT, typename RepT, typename PeriodT>
constexpr auto to_time(std::chrono::duration<RepT, PeriodT> d) noexcept
{
    using namespace std::chrono;
    return std::chrono::time_point<ClockT, Duration>{duration_cast<Duration>(d)};
}

template <typename ClockT>
constexpr auto to_time(timeval tv) noexcept
{
    using namespace std::chrono;
    return to_time<ClockT>(seconds{tv.tv_sec} + microseconds{tv.tv_usec});
}

template <typename ClockT>
constexpr auto to_time(timespec ts) noexcept
{
    using namespace std::chrono;
    return to_time<ClockT>(seconds{ts.tv_sec} + nanoseconds{ts.tv_nsec});
}

template <typename RepT, typename PeriodT>
constexpr timeval to_timeval(std::chrono::duration<RepT, PeriodT> d) noexcept
{
    using namespace std::chrono;
    const auto us = duration_cast<microseconds>(d).count();
    return {static_cast<time_t>(us / 1'000'000L), static_cast<suseconds_t>(us % 1'000'000L)};
}

template <typename ClockT>
constexpr timeval to_timeval(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    return to_timeval(time_since_epoch<ClockT, microseconds>(t));
}

template <typename RepT, typename PeriodT>
constexpr timespec to_timespec(std::chrono::duration<RepT, PeriodT> d) noexcept
{
    using namespace std::chrono;
    const auto ns = duration_cast<nanoseconds>(d).count();
    return {static_cast<time_t>(ns / 1'000'000'000L), static_cast<long>(ns % 1'000'000'000L)};
}

template <typename ClockT>
constexpr timespec to_timespec(std::chrono::time_point<ClockT, Duration> t) noexcept
{
    using namespace std::chrono;
    return to_timespec(time_since_epoch<ClockT, nanoseconds>(t));
}

template <typename DurationT>
struct PutTime {
    WallTime time;
    const char* fmt;
};

template <typename DurationT = Seconds>
auto put_time(WallTime t, const char* fmt) noexcept
{
    return PutTime<DurationT>{t, fmt};
}

template <typename DurationT>
std::ostream& operator<<(std::ostream& os, PutTime<DurationT> pt)
{
    const auto t = WallClock::to_time_t(pt.time);
    struct tm gmt;
    os << std::put_time(gmtime_r(&t, &gmt), pt.fmt);

    if constexpr (std::is_same_v<DurationT, Nanos>) {
        const auto ns = ns_since_epoch<WallClock>(pt.time);
        boost::io::ios_fill_saver ifs{os};
        boost::io::ios_width_saver iws{os};
        os << '.' << std::setfill('0') << std::setw(9) << (ns % 1'000'000'000L);
    } else if constexpr (std::is_same_v<DurationT, Micros>) {
        const auto us = us_since_epoch<WallClock>(pt.time);
        boost::io::ios_fill_saver ifs{os};
        boost::io::ios_width_saver iws{os};
        os << '.' << std::setfill('0') << std::setw(6) << (us % 1'000'000L);
    } else if constexpr (std::is_same_v<DurationT, Millis>) {
        const auto ms = ms_since_epoch<WallClock>(pt.time);
        boost::io::ios_fill_saver ifs{os};
        boost::io::ios_width_saver iws{os};
        os << '.' << std::setfill('0') << std::setw(3) << (ms % 1'000L);
    } else if constexpr (std::is_same_v<DurationT, Seconds>) {
    } else {
        static_assert(AlwaysFalse<DurationT>::value);
    }
    return os;
}

constexpr bool is_zero(timeval tv) noexcept
{
    return tv.tv_sec == 0 && tv.tv_usec == 0;
}

inline void clear(timeval& tv) noexcept
{
    tv.tv_sec = tv.tv_usec = 0;
}

inline timeval operator+(timeval lhs, timeval rhs) noexcept
{
    timeval tv;
    tv.tv_sec = lhs.tv_sec + rhs.tv_sec;
    tv.tv_usec = lhs.tv_usec + rhs.tv_usec;
    if (tv.tv_usec >= 1'000'000) {
        ++tv.tv_sec;
        tv.tv_usec -= 1'000'000;
    }
    return tv;
}

inline timeval operator-(timeval lhs, timeval rhs) noexcept
{
    timeval tv;
    tv.tv_sec = lhs.tv_sec - rhs.tv_sec;
    tv.tv_usec = lhs.tv_usec - rhs.tv_usec;
    if (tv.tv_usec < 0) {
        --tv.tv_sec;
        tv.tv_usec += 1'000'000;
    }
    return tv;
}

constexpr bool is_zero(timespec ts) noexcept
{
    return ts.tv_sec == 0 && ts.tv_nsec == 0;
}

inline void clear(timespec& ts) noexcept
{
    ts.tv_sec = ts.tv_nsec = 0;
}

inline timespec operator+(timespec lhs, timespec rhs) noexcept
{
    timespec ts;
    ts.tv_sec = lhs.tv_sec + rhs.tv_sec;
    ts.tv_nsec = lhs.tv_nsec + rhs.tv_nsec;
    if (ts.tv_nsec >= 1'000'000'000) {
        ++ts.tv_sec;
        ts.tv_nsec -= 1'000'000'000;
    }
    return ts;
}

inline timespec operator-(timespec lhs, timespec rhs) noexcept
{
    timespec ts;
    ts.tv_sec = lhs.tv_sec - rhs.tv_sec;
    ts.tv_nsec = lhs.tv_nsec - rhs.tv_nsec;
    if (ts.tv_nsec < 0) {
        --ts.tv_sec;
        ts.tv_nsec += 1'000'000'000;
    }
    return ts;
}

/// Parse fractional nanoseconds.
/// \param sv String of decimal digits.
/// \return nanoseconds.
constexpr auto parse_nanos(std::string_view sv) noexcept
{
    // clang-format off
    constexpr int c[] = {
                  0,
        100'000'000,
         10'000'000,
          1'000'000,
            100'000,
             10'000,
              1'000,
                100,
                 10,
                  1
    };
    // clang-format on

    // Truncate to ensure that we process no more than 9 decimal places.
    sv = sv.substr(0, 9);
    auto it = sv.begin(), end = sv.end();

    int ns{0};
    if (isdigit(*it)) {
        ns = *it++ - '0';
        while (it != end && isdigit(*it)) {
            ns *= 10;
            ns += *it++ - '0';
        }
    }
    return Nanos{ns * c[it - sv.begin()]};
}
static_assert(parse_nanos("000000001") == 1ns);

/// Time-only represented in UTC (Universal Time Coordinated, also known as "GMT") in either
/// HH:MM:SS (whole seconds) or HH:MM:SS.sss (milliseconds) format, colons, and period required.
/// This special-purpose field is paired with UTCDateOnly to form a proper UTCTimestamp for
/// bandwidth-sensitive messages. Valid values: HH = 00-23, MM = 00-59, SS = 00-5960 (60 only if UTC
/// leap second) (without milliseconds). HH = 00-23, MM = 00-59, SS = 00-5960 (60 only if UTC leap
/// second), sss=000-999 (indicating milliseconds).
constexpr std::optional<Nanos> parse_time_only(std::string_view sv) noexcept
{
    using namespace std::chrono;

    // clang-format off
    if (sv.size() < 8
        || !isdigit(sv[0])
        || !isdigit(sv[1])
        || sv[2] != ':'
        || !isdigit(sv[3])
        || !isdigit(sv[4])
        || sv[5] != ':'
        || !isdigit(sv[6])
        || !isdigit(sv[7])) {
        // Invalid format.
        return {};
    }
    // clang-format on
    const hours h{(sv[0] - '0') * 10 + sv[1] - '0'};
    const minutes m{(sv[3] - '0') * 10 + sv[4] - '0'};
    const seconds s{(sv[6] - '0') * 10 + sv[7] - '0'};
    Nanos ns{h + m + s};
    if (sv.size() > 8) {
        if (sv[8] != '.') {
            // Invalid delimiter.
            return {};
        }
        ns += parse_nanos(sv.substr(9));
    }
    return ns;
}
static_assert(*parse_time_only("12:00:00"sv) == 12h);

} // namespace sys
inline namespace util {

template <typename RepT, typename PeriodT>
struct TypeTraits<std::chrono::duration<RepT, PeriodT>> {
    static auto from_string(std::string_view sv) noexcept
    {
        using namespace std::chrono;
        using Duration = duration<RepT, PeriodT>;
        using Rep = typename Duration::rep;
        return Duration{TypeTraits<Rep>::from_string(sv)};
    }
    static auto from_string(const std::string& s) noexcept
    {
        return from_string(std::string_view{s});
    }
};

template <>
struct TypeTraits<WallTime> {
    static auto from_string(std::string_view sv) noexcept
    {
        return to_time<WallClock>(TypeTraits<Millis>::from_string(sv));
    }
    static auto from_string(const std::string& s) noexcept
    {
        return from_string(std::string_view{s});
    }
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_SYS_TIME_HPP
