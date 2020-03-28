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

#ifndef TOOLBOX_SYS_DATE_HPP
#define TOOLBOX_SYS_DATE_HPP

#include <toolbox/sys/Time.hpp>
#include <toolbox/util/IntTypes.hpp>

#include <cassert>

namespace toolbox {
inline namespace sys {

struct IsoDatePolicy : Int32Policy {
};
struct JDayPolicy : Int32Policy {
};

/// ISO8601 date in yyymmdd format.
using IsoDate = IntWrapper<IsoDatePolicy>;

constexpr auto operator""_ymd(unsigned long long val) noexcept
{
    return IsoDate{val};
}

/// Julian day.
using JDay = IntWrapper<JDayPolicy>;

constexpr auto operator""_jd(unsigned long long val) noexcept
{
    return JDay{val};
}

/// Gregorian to ISO8601 date.
constexpr auto ymd_to_iso(int year, int mon, int mday) noexcept
{
    assert(mon > 0 && mon <= 12);
    assert(mday > 0 && mday <= 31);
    return IsoDate{year * 10000 + mon * 100 + mday};
}

/// Gregorian date to Julian day.
constexpr auto ymd_to_jd(int year, int mon, int mday) noexcept
{
    // The formula given below was taken from the 1990 edition of the U.S. Naval Observatory's Almanac
    // for Computers.
    // See http://aa.usno.navy.mil/faq/docs/JD_Formula.php.

    const auto i = year;
    const auto j = mon;
    const auto k = mday;
    return JDay{k - 32075 + 1461 * (i + 4800 + (j - 14) / 12) / 4
                + 367 * (j - 2 - (j - 14) / 12 * 12) / 12
                - 3 * ((i + 4900 + (j - 14) / 12) / 100) / 4};
}

/// ISO8601 to Julian day.
constexpr auto iso_to_jd(IsoDate iso) noexcept
{
    const auto n = iso.count();
    const auto year = n / 10000;
    const auto mon = (n / 100 % 100);
    const auto mday = n % 100;
    return ymd_to_jd(year, mon, mday);
}

/// Julian day to ISO8601.
constexpr auto jd_to_iso(JDay jd) noexcept
{
    // The formula given above was taken from the 1990 edition of the U.S. Naval Observatory's
    // Almanac for Computers.
    // See http://aa.usno.navy.mil/faq/docs/JD_Formula.php.

    auto l = jd.count() + 68569;
    const auto n = 4 * l / 146097;
    l = l - (146097 * n + 3) / 4;
    auto i = 4000 * (l + 1) / 1461001;
    l = l - 1461 * i / 4 + 31;
    auto j = 80 * l / 2447;
    const auto k = l - 2447 * j / 80;
    l = j / 11;
    j = j + 2 - 12 * l;
    i = 100 * (n - 49) + i + l;

    return IsoDate{i * 10000 + j * 100 + k};
}

/// Juilian day to Modified Julian day. Epoch is November 17, 1858.
constexpr std::int32_t jd_to_mjd(JDay jd) noexcept
{
    return jd.count() - 2400000;
}

/// Modified Julian day to Julian day. Epoch is November 17, 1858.
constexpr auto mjd_to_jd(std::int32_t mjd) noexcept
{
    return JDay{mjd + 2400000};
}

/// Julian day to Truncated Julian day. Epoch is May 24, 1968.
constexpr std::int32_t jd_to_tjd(JDay jd) noexcept
{
    return jd.count() - 2440000;
}

/// Truncated Julian day to Julian day. Epoch is May 24, 1968.
constexpr JDay tjd_to_jd(std::int32_t tjd) noexcept
{
    return JDay{tjd + 2440000};
}

/// Julian day to Unix time.
constexpr WallTime jd_to_time(JDay jd) noexcept
{
    // Julian day for January 1st, 1970.
    const JDay jd_unix_epoc = 2440588_jd;
    const std::int64_t ms_in_day = 24 * 60 * 60 * 1000;
    // Add half day for 12pm.
    return to_time<WallClock>(Millis{(jd - jd_unix_epoc).count() * ms_in_day + (ms_in_day >> 1)});
}

/// Julian day to ISO8601 if argument is non-zero.
constexpr IsoDate maybe_jd_to_iso(JDay jd) noexcept
{
    return jd != 0_jd ? jd_to_iso(jd) : 0_ymd;
}

/// ISO8601 to Julian day if argument is non-zero.
constexpr JDay maybe_iso_to_jd(IsoDate iso) noexcept
{
    return iso != 0_ymd ? iso_to_jd(iso) : 0_jd;
}

constexpr bool is_week_day(JDay jday) noexcept
{
    return (jday.count() % 7) < 5;
}

constexpr bool is_weekend_day(JDay jday) noexcept
{
    return !is_week_day(jday);
}

/// Date represented in UTC (Universal Time Coordinated, also known as "GMT") in YYYYMMDD format.
/// This special-purpose field is paired with UTCTimeOnly to form a proper UTCTimestamp for
/// bandwidth-sensitive messages.
///
/// Valid values:
/// - YYYY = 0000-9999;
/// - MM = 01-12;
/// - DD = 01-31.
constexpr auto parse_date(std::string_view sv) noexcept
{
    return TypeTraits<IsoDate>::from_string(sv);
}
static_assert(parse_date("20180117") == 20180117_ymd);

/// Time/date combination represented in UTC (Universal Time Coordinated, also known as "GMT") in
/// either YYYYMMDD-HH:MM:SS (whole seconds) or YYYYMMDD-HH:MM:SS.sss (milliseconds) format, colons,
/// dash, and period required.
///
/// Valid values:
/// - YYYY = 0000-9999;
/// - MM = 01-12;
/// - DD = 01-31;
/// - HH = 00-23;
/// - MM = 00-59;
/// - SS = 00-60 (60 only if UTC leap second);
/// - sss = 000-999 (indicating milliseconds).
constexpr std::optional<WallTime> parse_time(std::string_view sv) noexcept
{
    if (sv.size() < 8) {
        // Date part is too short.
        return {};
    }
    const IsoDate d{parse_date(sv.substr(0, 8))};
    WallTime dt{};
    if (d != 0_ymd) {
        // Julian days start at noon.
        dt = jd_to_time(iso_to_jd(d)) - 12h;
    }
    // If there is a time component.
    if (sv.size() > 8) {
        if (sv[8] != '-' && sv[8] != 'T') {
            // Invalid delimiter.
            return {};
        }
        const auto t = parse_time_only(sv.substr(9));
        if (!t) {
            // Invalid time.
            return {};
        }
        dt += *t;
    }
    return dt;
}
static_assert(ms_since_epoch(*parse_time("20180824-05:32:29.123"sv)) == 1535088749123);

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_DATE_HPP
