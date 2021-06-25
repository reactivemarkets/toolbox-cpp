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

#include "Logger.hpp"

#include <toolbox/sys/Time.hpp>

#include <atomic>
#include <mutex>

#include <syslog.h>
#include <unistd.h> // getpid()

#include <sys/uio.h> // writev()

#if defined(__linux__)
#include <sys/syscall.h>
#endif

namespace toolbox {
inline namespace sys {
using namespace std;
namespace {

const char* Labels[] = {"CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

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
    void do_write_log(LogLevel level, LogMsgPtr&& msg, size_t size) noexcept override {}
} null_logger_;

class StdLogger final : public Logger {
    void do_write_log(LogLevel level, LogMsgPtr&& msg, size_t size) noexcept override
    {
        const auto now = WallClock::now();
        const auto t = WallClock::to_time_t(now);
        const auto ms = ms_since_epoch(now);

        struct tm tm; // NOLINT(hicpp-member-init)
        localtime_r(&t, &tm);

        // The following format has an upper-bound of 42 characters:
        // "%b %d %H:%M:%S.%03d %-7s [%d]: "
        //
        // Example:
        // Mar 14 00:00:00.000 WARNING [0123456789]: msg...
        // <---------------------------------------->
        char head[42 + 1];
        size_t hlen = strftime(head, sizeof(head), "%b %d %H:%M:%S", &tm);
        hlen += sprintf(head + hlen, ".%03d %-7s [%d]: ", static_cast<int>(ms % 1000),
                        log_label(level), static_cast<int>(gettid()));
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
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
        writev(fd, iov, sizeof(iov) / sizeof(iov[0]));
#pragma GCC diagnostic pop
    }
    mutex mutex_;
} std_logger_;

class SysLogger final : public Logger {
    void do_write_log(LogLevel level, LogMsgPtr&& msg, size_t size) noexcept override
    {
        int prio;
        switch (level) {
        case LogLevel::Crit:
            prio = LOG_CRIT;
            break;
        case LogLevel::Error:
            prio = LOG_ERR;
            break;
        case LogLevel::Warning:
            prio = LOG_WARNING;
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
    return Labels[static_cast<int>(min(max(level, LogLevel::Crit), LogLevel::Debug))];
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

void write_log(LogLevel level, LogMsgPtr&& msg, std::size_t size) noexcept
{
    acquire_logger().write_log(level, move(msg), size);
}

Logger::~Logger() = default;

} // namespace sys
} // namespace toolbox
