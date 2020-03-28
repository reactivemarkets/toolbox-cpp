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

#include "Log.hpp"

#include <toolbox/util/Finally.hpp>

#include <boost/test/unit_test.hpp>

#include <cstring>

using namespace std;
using namespace toolbox;

namespace {

template <typename T, typename U>
struct Foo {
    T first;
    U second;
};

ostream& operator<<(ostream& os, const Foo<int, int>& val)
{
    return os << '(' << val.first << ',' << val.second << ')';
}

int last_level{};
string last_msg{};

void test_logger(int level, string_view msg)
{
    last_level = level;
    last_msg.assign(msg.data(), msg.size());
}

} // namespace

BOOST_AUTO_TEST_SUITE(LogSuite)

BOOST_AUTO_TEST_CASE(LogLabelCase)
{
    BOOST_TEST(strcmp(log_label(-1), "CRIT") == 0);
    BOOST_TEST(strcmp(log_label(Log::Crit), "CRIT") == 0);
    BOOST_TEST(strcmp(log_label(Log::Error), "ERROR") == 0);
    BOOST_TEST(strcmp(log_label(Log::Warning), "WARNING") == 0);
    BOOST_TEST(strcmp(log_label(Log::Notice), "NOTICE") == 0);
    BOOST_TEST(strcmp(log_label(Log::Info), "INFO") == 0);
    BOOST_TEST(strcmp(log_label(Log::Debug), "DEBUG") == 0);
    BOOST_TEST(strcmp(log_label(99), "DEBUG") == 0);
}

BOOST_AUTO_TEST_CASE(LogMacroCase)
{
    auto prev_level = set_log_level(Log::Info);
    auto prev_logger = set_logger(test_logger);
    // clang-format off
    const auto finally = make_finally([prev_level, prev_logger]() noexcept {
        set_log_level(prev_level);
        set_logger(prev_logger);
    });
    // clang-format on

    TOOLBOX_LOG(Log::Info) << "test1: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Info);
    BOOST_TEST(last_msg == "test1: (10,20)");

    TOOLBOX_CRIT << "test2: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Crit);
    BOOST_TEST(last_msg == "test2: (10,20)");

    TOOLBOX_ERROR << "test3: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Error);
    BOOST_TEST(last_msg == "test3: (10,20)");

    TOOLBOX_WARNING << "test4: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Warning);
    BOOST_TEST(last_msg == "test4: (10,20)");

    TOOLBOX_NOTICE << "test5: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Notice);
    BOOST_TEST(last_msg == "test5: (10,20)");

    TOOLBOX_INFO << "test6: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Info);
    BOOST_TEST(last_msg == "test6: (10,20)");

    // This should not be logged.
    TOOLBOX_DEBUG << "test7: " << Foo<int, int>{10, 20};
    BOOST_TEST(last_level == Log::Info);
    BOOST_TEST(last_msg == "test6: (10,20)");
}

BOOST_AUTO_TEST_SUITE_END()
