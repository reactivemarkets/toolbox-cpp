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

#include "Buffer.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

void write(Buffer& buf, const char* data)
{
    const auto len = strlen(data);
    const auto out = buf.prepare(len);
    memcpy(static_cast<char*>(out.data()), data, len);
    buf.commit(len);
}

string read(Buffer& buf)
{
    const auto in = buf.data();
    const string s{static_cast<const char*>(in.data()), buffer_size(in)};
    buf.consume(buffer_size(in));
    return s;
}

string read(Buffer& buf, std::size_t limit)
{
    const auto in = buf.data(limit);
    const string s{static_cast<const char*>(in.data()), buffer_size(in)};
    buf.consume(buffer_size(in));
    return s;
}

} // namespace

BOOST_AUTO_TEST_SUITE(BufferSuite)

BOOST_AUTO_TEST_CASE(ReadWriteCase)
{
    Buffer buf;
    BOOST_CHECK(buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 0U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 0U);

    write(buf, "foo");
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 3U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 3U);
    BOOST_CHECK_EQUAL(memcmp(static_cast<const char*>(buf.data().data()), "foo", 3), 0);

    write(buf, "bar");
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 6U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 6U);
    BOOST_CHECK_EQUAL(memcmp(static_cast<const char*>(buf.data().data()), "foobar", 6), 0);

    BOOST_CHECK_EQUAL(read(buf, 4), "foob");
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 2U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 2U);
    BOOST_CHECK_EQUAL(memcmp(static_cast<const char*>(buf.data().data()), "ar", 2), 0);

    write(buf, "baz");
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 5U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 5U);
    BOOST_CHECK_EQUAL(memcmp(static_cast<const char*>(buf.data().data()), "arbaz", 5), 0);

    BOOST_CHECK_EQUAL(read(buf), "arbaz");
    BOOST_CHECK(buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 0U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 0U);
}

BOOST_AUTO_TEST_CASE(NoShrinkSmallCase)
{
    Buffer buf;

    // Avoid vector reallocation.
    buf.reserve(4096);
    // Base address.
    const auto* base = static_cast<const char*>(buf.data().data());

    // Simulated write.
    buf.prepare(16);
    buf.commit(16);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 16U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 16U);
    BOOST_CHECK_EQUAL(distance(base, static_cast<const char*>(buf.data().data())), 0);

    // Consume less than half.
    buf.consume(7);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 9U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 9U);
    BOOST_CHECK_EQUAL(distance(base, static_cast<const char*>(buf.data().data())), 7);

    // Consuming one more should not trigger shrink, because the buffer size is less than the shrink
    // threshold.
    buf.consume(1);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 8U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 8U);
    BOOST_CHECK_NE(distance(base, static_cast<const char*>(buf.data().data())), 0);
}

BOOST_AUTO_TEST_CASE(ShrinkLargeCase)
{
    Buffer buf;

    // Avoid vector reallocation.
    buf.reserve(4096);
    // Base address.
    const auto* base = static_cast<const char*>(buf.data().data());

    // Simulated write.
    buf.prepare(2048);
    buf.commit(2048);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 2048U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 2048U);
    BOOST_CHECK_EQUAL(static_cast<const char*>(buf.data().data()), base);

    // Consume less than half.
    buf.consume(1023);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 1025U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 1025U);
    BOOST_CHECK_EQUAL(static_cast<const char*>(buf.data().data()), base + 1023);

    // Consuming one more should not trigger shrink, because the buffer size is less than the shrink
    // threshold.
    buf.consume(1);
    BOOST_CHECK(!buf.empty());
    BOOST_CHECK_EQUAL(buf.size(), 1024U);
    BOOST_CHECK_EQUAL(buffer_size(buf.data()), 1024U);
    BOOST_CHECK_EQUAL(static_cast<const char*>(buf.data().data()), base);
}

BOOST_AUTO_TEST_SUITE_END()
