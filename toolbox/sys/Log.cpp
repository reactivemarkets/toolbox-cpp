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

#include <toolbox/sys/Time.hpp>

#include <algorithm> // max()
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

const char* labels_[] = {"CRIT", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};

// Global log level and logger function.
atomic<int> level_{Log::Info};
atomic<Logger> logger_{std_logger};
mutex mutex_;

inline int acquire_level() noexcept
{
    return level_.load(memory_order_acquire);
}

inline Logger acquire_logger() noexcept
{
    return logger_.load(memory_order_acquire);
}

thread_local LogMsg log_msg_;

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

} // namespace

const char* log_label(int level) noexcept
{
    return labels_[min<int>(max<int>(level, Log::Crit), Log::Debug)];
}

int get_log_level() noexcept
{
    return acquire_level();
}

int set_log_level(int level) noexcept
{
    return level_.exchange(max(level, 0), memory_order_acq_rel);
}

Logger get_logger() noexcept
{
    return acquire_logger();
}

Logger set_logger(Logger logger) noexcept
{
    return logger_.exchange(logger ? logger : null_logger, memory_order_acq_rel);
}

void write_log(int level, string_view msg) noexcept
{
    acquire_logger()(level, msg);
}

void null_logger(int level, string_view msg) noexcept {}

void std_logger(int level, string_view msg) noexcept
{
    const auto now = WallClock::now();
    const auto t = WallClock::to_time_t(now);
    const auto ms = ms_since_epoch(now);

    struct tm tm;
    localtime_r(&t, &tm);

    // The following format has an upper-bound of 42 characters:
    // "%b %d %H:%M:%S.%03d %-7s [%d]: "
    //
    // Example:
    // Mar 14 00:00:00.000 WARNING [0123456789]: msg...
    // <---------------------------------------->
    char head[42 + 1];
    size_t hlen = strftime(head, sizeof(head), "%b %d %H:%M:%S", &tm);
    hlen += sprintf(head + hlen, ".%03d %-7s [%d]: ", static_cast<int>(ms % 1000), log_label(level),
                    static_cast<int>(gettid()));
    char tail{'\n'};
    iovec iov[] = {
        {head, hlen},                                //
        {const_cast<char*>(msg.data()), msg.size()}, //
        {&tail, 1}                                   //
    };

    int fd{level > Log::Warning ? STDOUT_FILENO : STDERR_FILENO};
    // The following lock was required to avoid interleaving.
    lock_guard<mutex> lock{mutex_};
    // Best effort given that this is the logger.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    writev(fd, iov, sizeof(iov) / sizeof(iov[0]));
#pragma GCC diagnostic pop
}

void sys_logger(int level, string_view msg) noexcept
{
    int prio;
    switch (level) {
    case Log::Crit:
        prio = LOG_CRIT;
        break;
    case Log::Error:
        prio = LOG_ERR;
        break;
    case Log::Warning:
        prio = LOG_WARNING;
        break;
    case Log::Notice:
        prio = LOG_NOTICE;
        break;
    case Log::Info:
        prio = LOG_INFO;
        break;
    default:
        prio = LOG_DEBUG;
    }
    syslog(prio, "%.*s", static_cast<int>(msg.size()), msg.data());
}

LogMsg& log_msg() noexcept
{
    log_msg_.reset();
    return log_msg_;
}

} // namespace sys
} // namespace toolbox
