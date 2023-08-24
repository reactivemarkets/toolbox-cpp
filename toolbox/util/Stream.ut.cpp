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
    os << 12345678;
    BOOST_CHECK_EQUAL(os.size(), 7U);
    BOOST_CHECK_EQUAL(os.str(), "1234567");
    BOOST_CHECK(!os);

    os.reset();
    BOOST_CHECK(!!os);
    BOOST_CHECK_EQUAL((os << "test").str(), "test");
}

BOOST_AUTO_TEST_CASE(OStreamJoinerCase)
{
    array<string, 3> arr{{"foo", "bar", "baz"}};
    stringstream os;
    copy(arr.begin(), arr.end(), OStreamJoiner{os, ','});
    BOOST_CHECK_EQUAL(os.str(), "foo,bar,baz");
}

BOOST_AUTO_TEST_CASE(OStreamResetCase)
{
    OStaticStream<6> os{};
    // overflows the buffer, setting a bad flag
    os << "foobarbaz";
    BOOST_CHECK_EQUAL(os.data(), "foobar");
    // resets flag, allowing yes to be written
    os.reset();
    os << "yes";
    BOOST_CHECK_EQUAL(os.data(), "yesbar");
}

BOOST_AUTO_TEST_SUITE_END()
