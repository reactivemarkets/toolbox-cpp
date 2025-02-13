// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#include "Stream.hpp"

#include <boost/test/unit_test.hpp>

#include <array>
#include <sstream>
#include <limits>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(StreamSuite)

BOOST_AUTO_TEST_CASE(OStaticStreamCase)
{
    OStaticStream<7> os;
    BOOST_CHECK(os.empty());
    os << "foo";
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "foo");
    os << ',' << "bar";
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "foo,bar");

    os.reset();
    BOOST_CHECK(os.empty());
    os << 1234567;
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "1234567");
    BOOST_CHECK(os);

    // overflow
    os.reset();
    BOOST_CHECK(os.empty());
    os << 12345678;
    BOOST_CHECK_EQUAL(os.size(), 0U);
    BOOST_CHECK_EQUAL(os.str(), "");
    BOOST_CHECK(!os);

    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << float{1.2345};
    BOOST_CHECK_EQUAL(os.size(), 6U);
    BOOST_CHECK_EQUAL(os.str(), "1.2345");
    BOOST_CHECK(os);

    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << double{1.23456};
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "1.23456");
    BOOST_CHECK(os);

    // inf
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << std::numeric_limits<double>::infinity();
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "inf");
    BOOST_CHECK(os);

    // nan
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << std::numeric_limits<double>::quiet_NaN();
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "nan");
    BOOST_CHECK(os);

    // overflow
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << double{1.234567};
    BOOST_CHECK_EQUAL(os.size(), 0U);
    BOOST_CHECK_EQUAL(os.str(), "");
    BOOST_CHECK(!os);

    os << "test";
    BOOST_CHECK_EQUAL(os.str(), "test");
}

BOOST_AUTO_TEST_CASE(OStreamCase)
{
    OStream<7> os{OStream<7>::make_storage()};
    BOOST_CHECK(os.empty());
    os << "foo";
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "foo");
    os << ',' << "bar";
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "foo,bar");

    os.reset();
    BOOST_CHECK(os.empty());
    os << 1234567;
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "1234567");
    BOOST_CHECK(os);

    // overflow
    os.reset();
    BOOST_CHECK(os.empty());
    os << 12345678;
    BOOST_CHECK_EQUAL(os.size(), 0U);
    BOOST_CHECK_EQUAL(os.str(), "");
    BOOST_CHECK(!os);

    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << float{1.2345};
    BOOST_CHECK_EQUAL(os.size(), 6U);
    BOOST_CHECK_EQUAL(os.str(), "1.2345");
    BOOST_CHECK(os);

    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << double{1.23456};
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "1.23456");
    BOOST_CHECK(os);

    // inf
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << std::numeric_limits<double>::infinity();
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "inf");
    BOOST_CHECK(os);

    // nan
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << std::numeric_limits<double>::quiet_NaN();
    BOOST_CHECK_EQUAL(os.size(), 3U);
    BOOST_CHECK_EQUAL(os.str(), "nan");
    BOOST_CHECK(os);

    // overflow
    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK(os.empty());
    os << double{1.234567};
    BOOST_CHECK_EQUAL(os.size(), 0U);
    BOOST_CHECK_EQUAL(os.str(), "");
    BOOST_CHECK(!os);

    os << "test";
    BOOST_CHECK_EQUAL(os.str(), "test");
}

BOOST_AUTO_TEST_CASE(OStreamIteratorDelimCase)
{
    array<string, 3> arr{{"foo", "bar", "baz"}};
    stringstream os;
    copy(arr.begin(), arr.end(), OStreamIterator{os, ","});
    BOOST_CHECK_EQUAL(os.str(), "foo,bar,baz,");
    // Note, the comma at the end! This is correct as per
    // the spec of std::ostream_iterator. OStreamJoiner
    // behaves differently.
}

BOOST_AUTO_TEST_CASE(OStreamJoinerCase)
{
    array<string, 3> arr{{"foo", "bar", "baz"}};
    stringstream os;
    copy(arr.begin(), arr.end(), OStreamJoiner{os, ','});
    BOOST_CHECK_EQUAL(os.str(), "foo,bar,baz");
}

BOOST_AUTO_TEST_SUITE_END()
