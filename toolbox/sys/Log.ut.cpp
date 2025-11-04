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

template <typename StreamT>
    requires Streamable<StreamT>
StreamT& operator<<(StreamT& os, const Foo<int, int>& val)
{
    os << '(' << val.first << ',' << val.second << ')';
    return os;
}

struct TestLogger final : Logger {
    void do_write_log(WallTime /*ts*/, LogLevel level, int /*tid*/, LogMsgPtr&& msg,
                      size_t size) noexcept override
    {
        const auto finally = make_finally([&]() noexcept {
            log_buf_pool().bounded_push(std::move(msg));
        });
        last_level = level;
        last_msg.assign(static_cast<const char*>(msg.get()), size);
    }
    LogLevel last_level{};
    string last_msg{};
};

} // namespace

BOOST_AUTO_TEST_SUITE(LogSuite)

BOOST_AUTO_TEST_CASE(LogLabelCase)
{
    //NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel{-1}), "NONE"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::None), "NONE"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Crit), "CRIT"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Error), "ERROR"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Warn), "WARN"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Metric), "METRIC"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Notice), "NOTICE"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Info), "INFO"), 0);
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel::Debug), "DEBUG"), 0);
    //NOLINTNEXTLINE(clang-analyzer-optin.core.EnumCastOutOfRange)
    BOOST_CHECK_EQUAL(strcmp(log_label(LogLevel{99}), "DEBUG"), 0);
}

BOOST_AUTO_TEST_CASE(LogMacroCase)
{
    TestLogger tl;

    auto prev_level = set_log_level(LogLevel::Info);
    auto& prev_logger = set_logger(tl);
    // clang-format off
    const auto finally = make_finally([prev_level, &prev_logger]() noexcept {
        set_log_level(prev_level);
        set_logger(prev_logger);
    });
    // clang-format on

    TOOLBOX_LOG(LogLevel::Info) << "test1: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Info);
    BOOST_CHECK_EQUAL(tl.last_msg, "test1: (10,20)");

    TOOLBOX_CRIT << "test2: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Crit);
    BOOST_CHECK_EQUAL(tl.last_msg, "test2: (10,20)");

    TOOLBOX_ERROR << "test3: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Error);
    BOOST_CHECK_EQUAL(tl.last_msg, "test3: (10,20)");

    TOOLBOX_WARN << "test4: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Warn);
    BOOST_CHECK_EQUAL(tl.last_msg, "test4: (10,20)");

    TOOLBOX_METRIC << "test5: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Metric);
    BOOST_CHECK_EQUAL(tl.last_msg, "test5: (10,20)");

    TOOLBOX_NOTICE << "test6: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Notice);
    BOOST_CHECK_EQUAL(tl.last_msg, "test6: (10,20)");

    TOOLBOX_INFO << "test7: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Info);
    BOOST_CHECK_EQUAL(tl.last_msg, "test7: (10,20)");

    // This should not be logged.
    TOOLBOX_DEBUG << "test8: " << Foo<int, int>{10, 20};
    BOOST_CHECK_EQUAL(tl.last_level, LogLevel::Info);
    BOOST_CHECK_EQUAL(tl.last_msg, "test7: (10,20)");
}

BOOST_AUTO_TEST_SUITE_END()
