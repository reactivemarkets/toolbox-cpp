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

#include "Time.hpp"

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

BOOST_AUTO_TEST_SUITE(TimeSuite)

BOOST_AUTO_TEST_CASE(TimeParseNanosCase)
{
    BOOST_TEST(parse_nanos("") == 0ns);
    BOOST_TEST(parse_nanos("0") == 0ns);
    BOOST_TEST(parse_nanos("1") == 100ms);

    BOOST_TEST(parse_nanos("000000001") == 1ns);
    BOOST_TEST(parse_nanos("000000012") == 12ns);
    BOOST_TEST(parse_nanos("000000123") == 123ns);

    BOOST_TEST(parse_nanos("000001") == 1us);
    BOOST_TEST(parse_nanos("000012") == 12us);
    BOOST_TEST(parse_nanos("000123") == 123us);

    BOOST_TEST(parse_nanos("001") == 1ms);
    BOOST_TEST(parse_nanos("012") == 12ms);
    BOOST_TEST(parse_nanos("123") == 123ms);

    BOOST_TEST(parse_nanos("123456789999") == 123456789ns);
}

BOOST_AUTO_TEST_CASE(TimeParseTimeOnlyCase)
{
    BOOST_TEST(!parse_time_only(""sv));
    BOOST_TEST(!parse_time_only("12:34:5"sv));
    BOOST_TEST(!parse_time_only("x2:34:56"sv));
    BOOST_TEST(!parse_time_only("1x:34:56"sv));
    BOOST_TEST(!parse_time_only("12x34:56"sv));
    BOOST_TEST(!parse_time_only("12:x4:56"sv));
    BOOST_TEST(!parse_time_only("12:3x:56"sv));
    BOOST_TEST(!parse_time_only("12:34x56"sv));
    BOOST_TEST(!parse_time_only("12:34:x6"sv));
    BOOST_TEST(!parse_time_only("12:34:5x"sv));

    BOOST_TEST(*parse_time_only("00:00:00"sv) == 0ns);
    BOOST_TEST(*parse_time_only("12:00:00"sv) == 12h);
    BOOST_TEST(*parse_time_only("00:34:00"sv) == 34min);
    BOOST_TEST(*parse_time_only("00:00:56"sv) == 56s);

    BOOST_TEST(*parse_time_only("00:00:56."sv) == 56s);
    BOOST_TEST(*parse_time_only("00:00:00.789"sv) == 789ms);
    BOOST_TEST(*parse_time_only("00:00:00.000789"sv) == 789us);
    BOOST_TEST(*parse_time_only("00:00:00.000000789"sv) == 789ns);
}

BOOST_AUTO_TEST_SUITE_END()
