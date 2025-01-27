// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Time.hpp"
#include "Date.hpp"

#include <boost/test/unit_test.hpp>
#include <sstream>

namespace std::chrono {
template <typename RepT, typename PeriodT>
ostream& operator<<(ostream& os, duration<RepT, PeriodT> d)
{
    return os << d.count();
}
} // namespace std::chrono

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(TimeSuite)

BOOST_AUTO_TEST_CASE(TimeParseNanosCase)
{
    BOOST_CHECK_EQUAL(parse_nanos(""), 0ns);
    BOOST_CHECK_EQUAL(parse_nanos("0"), 0ns);
    BOOST_CHECK_EQUAL(parse_nanos("1"), 100ms);

    BOOST_CHECK_EQUAL(parse_nanos("000000001"), 1ns);
    BOOST_CHECK_EQUAL(parse_nanos("000000012"), 12ns);
    BOOST_CHECK_EQUAL(parse_nanos("000000123"), 123ns);

    BOOST_CHECK_EQUAL(parse_nanos("000001"), 1us);
    BOOST_CHECK_EQUAL(parse_nanos("000012"), 12us);
    BOOST_CHECK_EQUAL(parse_nanos("000123"), 123us);

    BOOST_CHECK_EQUAL(parse_nanos("001"), 1ms);
    BOOST_CHECK_EQUAL(parse_nanos("012"), 12ms);
    BOOST_CHECK_EQUAL(parse_nanos("123"), 123ms);

    BOOST_CHECK_EQUAL(parse_nanos("123456789999"), 123456789ns);
}

BOOST_AUTO_TEST_CASE(TimeParseTimeOnlyCase)
{
    BOOST_CHECK(!parse_time_only(""sv));
    BOOST_CHECK(!parse_time_only("12:34:5"sv));
    BOOST_CHECK(!parse_time_only("x2:34:56"sv));
    BOOST_CHECK(!parse_time_only("1x:34:56"sv));
    BOOST_CHECK(!parse_time_only("12x34:56"sv));
    BOOST_CHECK(!parse_time_only("12:x4:56"sv));
    BOOST_CHECK(!parse_time_only("12:3x:56"sv));
    BOOST_CHECK(!parse_time_only("12:34x56"sv));
    BOOST_CHECK(!parse_time_only("12:34:x6"sv));
    BOOST_CHECK(!parse_time_only("12:34:5x"sv));

    BOOST_CHECK_EQUAL(*parse_time_only("00:00:00"sv), 0ns);
    BOOST_CHECK_EQUAL(*parse_time_only("12:00:00"sv), 12h);
    BOOST_CHECK_EQUAL(*parse_time_only("00:34:00"sv), 34min);
    BOOST_CHECK_EQUAL(*parse_time_only("00:00:56"sv), 56s);

    BOOST_CHECK_EQUAL(*parse_time_only("00:00:56."sv), 56s);
    BOOST_CHECK_EQUAL(*parse_time_only("00:00:00.789"sv), 789ms);
    BOOST_CHECK_EQUAL(*parse_time_only("00:00:00.000789"sv), 789us);
    BOOST_CHECK_EQUAL(*parse_time_only("00:00:00.000000789"sv), 789ns);
}

BOOST_AUTO_TEST_CASE(PutTimeOutput)
{
    std::stringstream stream;

    auto tm = parse_time("20180824T05:32:29.123456789"sv);
    BOOST_CHECK(tm.has_value());

    stream << put_time<Seconds>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29");

    stream.str("");
    stream << put_time<Millis>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.123");

    stream.str("");
    stream << put_time<Micros>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.123456");

    stream.str("");
    stream << put_time<Nanos>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.123456789");
}

BOOST_AUTO_TEST_CASE(PutTimeOutput2)
{
    std::stringstream stream;

    auto tm = parse_time("20180824T05:32:29.001001001"sv);
    BOOST_CHECK(tm.has_value());

    stream << put_time<Seconds>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29");

    stream.str("");
    stream << put_time<Millis>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.001");

    stream.str("");
    stream << put_time<Micros>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.001001");

    stream.str("");
    stream << put_time<Nanos>(*tm, "%Y%m%dT%H:%M:%S");
    BOOST_CHECK_EQUAL(stream.str(), "20180824T05:32:29.001001001");
}

BOOST_AUTO_TEST_SUITE_END()
