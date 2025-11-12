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

#include <toolbox/util/Finally.hpp>

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
    struct S {
        pid_t tid = {};
        bool init_done = false;
    };

    thread_local S s{};

    // 'tid' cannot be set during initialisation of struct S -- because
    // the C++ standard doesn't guarantee which thread initialises it.
    if (!s.init_done) [[unlikely]] {
        s.tid = syscall(SYS_gettid);
        s.init_done = true;
    } 

    return s.tid;
}
#else
inline pid_t gettid()
{
    return getpid();
}
#endif

struct LogBufPoolWrapper {
    static constexpr std::size_t InitialPoolSize = 8;
    LogBufPoolWrapper()
    {
        for ([[maybe_unused]] std::size_t i = 0; i < InitialPoolSize; i++) {
            pool.bounded_push(util::make_storage<MaxLogLine>());
        }
    }
    LogBufPool pool;
};
static LogBufPoolWrapper log_buf_pool_{};

class NullLogger final : public Logger {
    void do_write_log(WallTime /*ts*/, LogLevel /*level*/, int /*tid*/, LogMsgPtr&& msg,
                      size_t /*size*/) noexcept override
    {
        log_buf_pool().bounded_push(std::move(msg));
    }
} null_logger_;

class StdLogger final : public Logger {
    void do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                      size_t size) noexcept override
    {
        const auto finally = make_finally([&]() noexcept {
            log_buf_pool().bounded_push(std::move(msg));
        });

        const auto t{WallClock::to_time_t(ts)};
        tm tm;
        localtime_r(&t, &tm);

        // The following format has an upper-bound of 49 characters:
        // "%Y/%m/%d %H:%M:%S.%06d %-6s [%d]: "
        //
        // Example:
        // 2022/03/14 00:00:00.000000 NOTICE [0123456789]: msg...
        // <---------------------------------------------->
        static constexpr size_t upper_bound{48 + 1};
        char head[upper_bound];
        size_t hlen{strftime(head, sizeof(head), "%Y/%m/%d %H:%M:%S", &tm)};
        const auto us{static_cast<int>(us_since_epoch(ts) % 1000000)};
        hlen += snprintf(head + hlen, upper_bound - hlen, ".%06d %-6s [%d]: ", us, log_label(level), tid);
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
        const auto finally = make_finally([&]() noexcept {
            log_buf_pool().bounded_push(std::move(msg));
        });

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
        syslog(prio, "%.*s", static_cast<int>(size), msg.get());
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

LogBufPool& log_buf_pool() noexcept
{
    return log_buf_pool_.pool;
}

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
    int fake_cnt = 0;

    while (tq_.pop(t)) {
        if (t.msg != nullptr) {
            logger_.write_log(t.ts, t.level, t.tid, LogMsgPtr{t.msg}, t.size);
        } else {
            fake_cnt++;
        }
    }
    fake_pushed_count_.fetch_sub(fake_cnt, std::memory_order_relaxed);
}

bool AsyncLogger::run()
{
    write_all_messages();
    std::this_thread::sleep_for(50ms);

    return (!tq_.empty() || !stop_);
}

void AsyncLogger::stop()
{
    stop_ = true;
}

void AsyncLogger::set_warming_mode(bool enable) noexcept
{
    warming_mode_enabled_ = enable;
}

void AsyncLogger::do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                               size_t size) noexcept
{
    char* const msg_ptr = msg.release();
    auto push_to_queue = [&](char* ptr) {
        return tq_.push(Task{.ts = ts, .level = level, .tid = tid, .msg = ptr, .size = size});
    };
    try {
        if (warming_mode_enabled_) [[unlikely]] {
            const auto d = ts - last_time_fake_pushed_;
            const auto cnt = fake_pushed_count_.load(std::memory_order_relaxed);

            constexpr Millis FakePushInterval = 10ms;
            constexpr int MaxPushedFakeCount = 25;

            if (duration_cast<Millis>(d) >= FakePushInterval && cnt < MaxPushedFakeCount) {
                push_to_queue(nullptr);
                last_time_fake_pushed_ = ts;
                fake_pushed_count_.fetch_add(1, std::memory_order_relaxed);
            }
        } else if (push_to_queue(msg_ptr)) [[likely]] {
            // Successfully pushed the task to the queue, release ownership of msg_ptr.
            return;
        }
    } catch (const std::bad_alloc&) {
        // Catching `std::bad_alloc` here is *defensive plumbing* that keeps the logger non-throwing
        // and prevents crashes caused by an out-of-memory situation during rare log-burst spikes.
    }
    // Failed to push the task, restore ownership of msg_ptr.
    log_buf_pool().bounded_push(LogMsgPtr{msg_ptr});
}

} // namespace sys
} // namespace toolbox
