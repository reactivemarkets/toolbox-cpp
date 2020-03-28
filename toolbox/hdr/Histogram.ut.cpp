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

#include "Histogram.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(HistogramSuite)

constexpr std::int64_t Lowest{1};
constexpr std::int64_t Highest{3600ull * 1000 * 1000};
constexpr std::int32_t Significant{3};
constexpr auto TestValueLevel = 4;
constexpr auto Interval = 10000;
constexpr auto Bitness = 64;

BOOST_AUTO_TEST_CASE(HistogramBasicCase)
{
    HdrHistogram h{Lowest, Highest, Significant};
    const auto expected_bucket_count = Bitness == 64 ? 22 : 21;
    const auto expected_counts_len = Bitness == 64 ? 23552 : 22528;

    BOOST_TEST(h.lowest_trackable_value() == Lowest);
    BOOST_TEST(h.highest_trackable_value() == Highest);
    BOOST_TEST(h.significant_figures() == Significant);
    BOOST_TEST(h.sub_bucket_count() == 2048);
    BOOST_TEST(h.bucket_count() == expected_bucket_count);
    BOOST_TEST(h.counts_len() == expected_counts_len);
}

BOOST_AUTO_TEST_CASE(HistogramEmptyCase)
{
    HdrHistogram h{1, 100000000, 1};
    BOOST_TEST(h.min() == numeric_limits<int64_t>::max());
    BOOST_TEST(h.max() == 0);
}

BOOST_AUTO_TEST_CASE(HistogramRecordValueCase)
{
    HdrHistogram h{Lowest, Highest, Significant};
    h.record_value(TestValueLevel);
    BOOST_TEST(h.count_at_value(TestValueLevel) == 1);
    BOOST_TEST(h.total_count() == 1);
}

BOOST_AUTO_TEST_CASE(HistogramInvalidSigFigCase)
{
    BOOST_CHECK_THROW(HdrHistogram(1, 36000000, -1), invalid_argument);
    BOOST_CHECK_THROW(HdrHistogram(1, 36000000, 0), invalid_argument);
    BOOST_CHECK_THROW(HdrHistogram(1, 36000000, 6), invalid_argument);
}

BOOST_AUTO_TEST_CASE(HistogramInvalidInitCase)
{
    BOOST_CHECK_THROW(HdrHistogram(0, 64 * 1024, 2), invalid_argument);
    BOOST_CHECK_THROW(HdrHistogram(80, 110, 5), invalid_argument);
}

BOOST_AUTO_TEST_CASE(HistogramOutOfRangeCase)
{
    HdrHistogram h{1, 1000, 4};
    BOOST_TEST(h.record_value(32767));
    BOOST_TEST(!h.record_value(32768));
}

BOOST_AUTO_TEST_CASE(HistogramResetCase)
{
    HdrHistogram h{1, 10000000, 3};
    for (int i{0}; i < 1000; ++i) {
        BOOST_TEST(h.record_value(i));
    }
    h.reset();
    BOOST_TEST(h.max() == 0);
}

BOOST_AUTO_TEST_CASE(HistogramTotalCountCase)
{
    HdrHistogram h{1, 10000000, 3};
    for (int i{0}; i < 1000; ++i) {
        BOOST_TEST(h.record_value(i));
        BOOST_TEST(h.total_count() == i + 1);
    }
}

BOOST_AUTO_TEST_CASE(HistogramRecordEquivalentValueCase)
{
    HdrHistogram h{Lowest, Highest, Significant};
    BOOST_TEST(8183 * 1024 + 1023 == h.highest_equivalent_value(8180 * 1024));
    BOOST_TEST(8191 * 1024 + 1023 == h.highest_equivalent_value(8191 * 1024));
    BOOST_TEST(8199 * 1024 + 1023 == h.highest_equivalent_value(8193 * 1024));
    BOOST_TEST(9999 * 1024 + 1023 == h.highest_equivalent_value(9995 * 1024));
    BOOST_TEST(10007 * 1024 + 1023 == h.highest_equivalent_value(10007 * 1024));
    BOOST_TEST(10015 * 1024 + 1023 == h.highest_equivalent_value(10008 * 1024));
}

BOOST_AUTO_TEST_SUITE_END()
