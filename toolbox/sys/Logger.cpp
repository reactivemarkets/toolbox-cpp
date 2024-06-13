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

#include "Logger.hpp"

#include <atomic>
#include <mutex>

#include <syslog.h>

#include <sys/uio.h> // writev()

#if defined(__linux__)
#include <sys/syscall.h>
#endif

namespace toolbox {
inline namespace sys {
using namespace std;
namespace {

const char* Labels[] = {"NONE", "CRIT", "ERROR", "WARN", "METRIC", "NOTICE", "INFO", "DEBUG"};

// The gettid() function is a Linux-specific function call.
#if defined(__linux__)
inline pid_t gettid()
{
    return syscall(SYS_gettid);
}
#else
inline pid_t gettid()
{
    return getpid();
}
#endif

class NullLogger final : public Logger {
    void do_write_log(WallTime /*ts*/, LogLevel /*level*/, int /*tid*/, LogMsgPtr&& /*msg*/,
                      size_t /*size*/) noexcept override
    {
    }
} null_logger_;

class StdLogger final : public Logger {
    void do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                      size_t size) noexcept override
    {
        const auto t{WallClock::to_time_t(ts)};
        tm tm;
        localtime_r(&t, &tm);

        // The following format has an upper-bound of 49 characters:
        // "%Y/%m/%d %H:%M:%S.%06d %-6s [%d]: "
        //
        // Example:
        // 2022/03/14 00:00:00.000000 NOTICE [0123456789]: msg...
        // <---------------------------------------------->
        char head[48 + 1];
        size_t hlen{strftime(head, sizeof(head), "%Y/%m/%d %H:%M:%S", &tm)};
        const auto us{static_cast<int>(us_since_epoch(ts) % 1000000)};
        hlen += sprintf(head + hlen, ".%06d %-6s [%d]: ", us, log_label(level), tid);
        char tail{'\n'};
        iovec iov[] = {
            {head, hlen},      //
            {msg.get(), size}, //
            {&tail, 1}         //
        };

        int fd{level > LogLevel::Error ? STDOUT_FILENO : STDERR_FILENO};
        // The following lock was required to avoid interleaving.
        lock_guard<mutex> lock{mutex_};
        // Best effort given that this is the logger.
        ignore = writev(fd, iov, sizeof(iov) / sizeof(iov[0]));
    }
    mutex mutex_;
} std_logger_;

class SysLogger final : public Logger {
    void do_write_log(WallTime /*ts*/, LogLevel level, int /*tid*/, LogMsgPtr&& msg,
                      size_t size) noexcept override
    {
        int prio;
        switch (level) {
        case LogLevel::None:
            return;
        case LogLevel::Crit:
            prio = LOG_CRIT;
            break;
        case LogLevel::Error:
            prio = LOG_ERR;
            break;
        case LogLevel::Warn:
            prio = LOG_WARNING;
            break;
        case LogLevel::Metric:
            prio = LOG_NOTICE;
            break;
        case LogLevel::Notice:
            prio = LOG_NOTICE;
            break;
        case LogLevel::Info:
            prio = LOG_INFO;
            break;
        default:
            prio = LOG_DEBUG;
        }
        syslog(prio, "%.*s", static_cast<int>(size), static_cast<const char*>(msg.get()));
    }
} sys_logger_;

// Global log level and logger function.
atomic<LogLevel> level_{LogLevel::Info};
atomic<Logger*> logger_{&std_logger_};

inline LogLevel acquire_level() noexcept
{
    return level_.load(memory_order_acquire);
}

inline Logger& acquire_logger() noexcept
{
    return *logger_.load(memory_order_acquire);
}

} // namespace

Logger& null_logger() noexcept
{
    return null_logger_;
}

Logger& std_logger() noexcept
{
    return std_logger_;
}

Logger& sys_logger() noexcept
{
    return sys_logger_;
}

const char* log_label(LogLevel level) noexcept
{
    return Labels[static_cast<int>(min(max(level, LogLevel::None), LogLevel::Debug))];
}

LogLevel get_log_level() noexcept
{
    return acquire_level();
}

LogLevel set_log_level(LogLevel level) noexcept
{
    return level_.exchange(max(level, LogLevel{}), memory_order_acq_rel);
}

Logger& get_logger() noexcept
{
    return acquire_logger();
}

Logger& set_logger(Logger& logger) noexcept
{
    return *logger_.exchange(&logger, memory_order_acq_rel);
}

void write_log(WallTime ts, LogLevel level, LogMsgPtr&& msg, std::size_t size) noexcept
{
    acquire_logger().write_log(ts, level, static_cast<int>(gettid()), std::move(msg), size);
}

Logger::~Logger() = default;

AsyncLogger::AsyncLogger(Logger& logger)
: logger_{logger}
{
}

AsyncLogger::~AsyncLogger()
{
    write_all_messages();
}

void AsyncLogger::write_all_messages()
{
    Task t;
    while (tq_.pop(t)) {
        logger_.write_log(t.ts, t.level, t.tid, LogMsgPtr{t.msg}, t.size);
    }
}

bool AsyncLogger::run()
{
    write_all_messages();
    std::this_thread::sleep_for(25ms);

    return (!tq_.empty() || !stop_);
}

void AsyncLogger::stop()
{
    stop_ = true;
}

void AsyncLogger::do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                               size_t size) noexcept
{
    // if the queue is full, skip the message
    if (void* msg_ptr = msg.release();
        !tq_.push(Task{.ts = ts, .level = level, .tid = tid, .msg = msg_ptr, .size = size})) {
        LogMsgPtr{msg_ptr};
    }
}

} // namespace sys
} // namespace toolbox
