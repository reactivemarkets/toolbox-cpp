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

#include "Date.hpp"

#include <boost/test/unit_test.hpp>

namespace std::chrono {
template <typename RepT, typename PeriodT>
ostream& operator<<(ostream& os, duration<RepT, PeriodT> d)
{
    return os << d.count();
}
} // namespace std::chrono

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(DateSuite)

BOOST_AUTO_TEST_CASE(YmdToIsoCase)
{
    BOOST_TEST(20140314_ymd == ymd_to_iso(2014, 3, 14));
}

BOOST_AUTO_TEST_CASE(YmdToJdCase)
{
    BOOST_TEST(2456731_jd == ymd_to_jd(2014, 3, 14));
    // AD 1978 January 1, 0h UT is JD 2443509.5 and AD 1978 July 21, 15h UT, is JD 2443711.125.
    BOOST_TEST(2443510_jd == ymd_to_jd(1978, 1, 1));
    BOOST_TEST(2443711_jd == ymd_to_jd(1978, 7, 21));
}

BOOST_AUTO_TEST_CASE(JdToIsoCase)
{
    BOOST_TEST(20140314_ymd == jd_to_iso(ymd_to_jd(2014, 3, 14)));
}

BOOST_AUTO_TEST_CASE(JdToTimeCase)
{
    BOOST_TEST(1394798400000ms == jd_to_time(ymd_to_jd(2014, 3, 14)).time_since_epoch());
}

BOOST_AUTO_TEST_CASE(ParseDateCase)
{
    BOOST_TEST(parse_date("20180820"sv) == IsoDate{20180820});
}

BOOST_AUTO_TEST_CASE(TimeParseTimeCase)
{
    BOOST_TEST(!parse_time(""sv));
    BOOST_TEST(!parse_time("20180824x05:32:29.123"sv));
    BOOST_TEST(!parse_time("20180824-05x32:29.123"sv));

    BOOST_TEST(ms_since_epoch(*parse_time("00000000-00:00:00.000"sv)) == 0);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824"sv)) == 1535068800000);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824-05:32:29"sv)) == 1535088749000);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824-05:32:29.123"sv)) == 1535088749123);
    BOOST_TEST(us_since_epoch(*parse_time("20180824-05:32:29.123456"sv)) == 1535088749123456);

    BOOST_TEST(ms_since_epoch(*parse_time("00000000T00:00:00.000"sv)) == 0);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824"sv)) == 1535068800000);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824T05:32:29"sv)) == 1535088749000);
    BOOST_TEST(ms_since_epoch(*parse_time("20180824T05:32:29.123"sv)) == 1535088749123);
    BOOST_TEST(us_since_epoch(*parse_time("20180824T05:32:29.123456"sv)) == 1535088749123456);
}

BOOST_AUTO_TEST_SUITE_END()
