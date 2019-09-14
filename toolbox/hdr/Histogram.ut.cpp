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

#include "Histogram.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace utf = boost::unit_test;

BOOST_AUTO_TEST_SUITE(HistogramSuite)

constexpr auto Lowest = 1;
constexpr std::int64_t Highest = 3600ull * 1000 * 1000;
constexpr auto Significant = 3;
constexpr auto TestValueLevel = 4;
constexpr auto Interval = 10000;
constexpr auto Bitness = 64;

BOOST_AUTO_TEST_CASE(HdrHistogramBasicCase)
{
    HdrHistogram hist{Lowest, Highest, Significant};
    const auto expected_bucket_count = Bitness == 64 ? 22 : 21;
    const auto expected_counts_len = Bitness == 64 ? 23552 : 22528;

    BOOST_TEST(hist.bucket_count == expected_bucket_count);
    BOOST_TEST(hist.sub_bucket_count == 2048);
    BOOST_TEST(hist.counts_len == expected_counts_len);
    BOOST_TEST(hist.unit_magnitude == 0);
    BOOST_TEST(hist.sub_bucket_half_count_magnitude == 10);
}

BOOST_AUTO_TEST_CASE(HdrHistogramEmptyCase)
{
    HdrHistogram hist{1, 100000000, 1};

    BOOST_TEST(hist.min() == 0);
    BOOST_TEST(hist.max() == 0);
    BOOST_TEST(hist.mean() == 0);
    BOOST_TEST(hist.stddev() == 0);
}

BOOST_AUTO_TEST_CASE(HdrHistogramRecordValueCase)
{
    HdrHistogram hist{Lowest, Highest, Significant};
    hist.record(TestValueLevel);
    BOOST_TEST(hist.get_count_at_value(TestValueLevel) == 1);
    BOOST_TEST(hist.get_total_count() == 1);
}

BOOST_AUTO_TEST_CASE(HdrHistogramRecordEquivalentValueCase)
{
    HdrHistogram histogram{Lowest, Highest, Significant};
    BOOST_TEST(8183 * 1024 + 1023 == histogram.get_highest_equivalent_value(8180 * 1024));
    BOOST_TEST(8191 * 1024 + 1023 == histogram.get_highest_equivalent_value(8191 * 1024));
    BOOST_TEST(8199 * 1024 + 1023 == histogram.get_highest_equivalent_value(8193 * 1024));
    BOOST_TEST(9999 * 1024 + 1023 == histogram.get_highest_equivalent_value(9995 * 1024));
    BOOST_TEST(10007 * 1024 + 1023 == histogram.get_highest_equivalent_value(10007 * 1024));
    BOOST_TEST(10015 * 1024 + 1023 == histogram.get_highest_equivalent_value(10008 * 1024));
}

BOOST_AUTO_TEST_CASE(HdrHistogramLargeNumbersCase)
{
    HdrHistogram hist{20000000, 100000000, 5};
    hist.record(100000000);
    hist.record(20000000);
    hist.record(30000000);

    BOOST_TEST(hist.values_are_equivalent(hist.percentile(50.0), 20000000));
    BOOST_TEST(hist.values_are_equivalent(hist.percentile(83.33), 30000000));
    BOOST_TEST(hist.values_are_equivalent(hist.percentile(83.34), 100000000));
    BOOST_TEST(hist.values_are_equivalent(hist.percentile(99.0), 100000000));
}

BOOST_AUTO_TEST_SUITE_END()
