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

#ifndef TOOLBOX_SYS_LOGGER_HPP
#define TOOLBOX_SYS_LOGGER_HPP

#include <boost/lockfree/queue.hpp>
#include <thread>

#include <toolbox/sys/Limits.hpp>
#include <toolbox/sys/Time.hpp>
#include <toolbox/util/Storage.hpp>
#include <toolbox/util/Concepts.hpp>

namespace toolbox {
inline namespace sys {

class Logger;

enum class LogLevel : int {
    /// None.
    None,
    /// Critical.
    Crit,
    /// Error.
    Error,
    /// Warning.
    Warn,
    //// Metric.
    Metric,
    /// Notice.
    Notice,
    /// Information.
    Info,
    /// Debug.
    Debug
};

using LogMsgPtr = StoragePtr<MaxLogLine>;

/// Null logger. This logger does nothing and is effectively /dev/null.
TOOLBOX_API Logger& null_logger() noexcept;

/// Standard logger. This logger writes to stdout if the log level is greater than LogWarn, and
/// stdout otherwise.
TOOLBOX_API Logger& std_logger() noexcept;

/// System logger. This logger calls syslog().
TOOLBOX_API Logger& sys_logger() noexcept;

/// Return log label for given log level.
TOOLBOX_API const char* log_label(LogLevel level) noexcept;

/// Return current log level.
TOOLBOX_API LogLevel get_log_level() noexcept;

/// Return true if level is less than or equal to current log level.
inline bool is_log_level(LogLevel level) noexcept
{
    return level <= get_log_level();
}

/// Set log level globally for all threads.
TOOLBOX_API LogLevel set_log_level(LogLevel level) noexcept;

/// Return current logger.
TOOLBOX_API Logger& get_logger() noexcept;

/// Set logger globally for all threads.
TOOLBOX_API Logger& set_logger(Logger& logger) noexcept;

inline Logger& set_logger(std::nullptr_t) noexcept
{
    return set_logger(null_logger());
}

/// Unconditionally write log message to the logger. Specifically, this function does not check that
/// level is allowed by the current log level; users are expected to call is_log_level() first,
/// before formatting the log message.
TOOLBOX_API void write_log(WallTime ts, LogLevel level, LogMsgPtr&& msg, std::size_t size) noexcept;

/// The Logger is implemented by types that may be woken-up, interrupted or otherwise notified
/// asynchronously.
class TOOLBOX_API Logger {
  public:
    Logger() noexcept = default;
    virtual ~Logger();

    // Copy.
    Logger(const Logger&) noexcept = default;
    Logger& operator=(const Logger&) noexcept = default;

    // Move.
    Logger(Logger&&) noexcept = default;
    Logger& operator=(Logger&&) noexcept = default;

    void write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg, std::size_t size) noexcept
    {
        do_write_log(ts, level, tid, std::move(msg), size);
    }

  protected:
    virtual void do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                              std::size_t size) noexcept
        = 0;
};

class TOOLBOX_API AsyncLogger : public Logger {
    struct Task {
        WallTime ts;
        LogLevel level;
        int tid;
        char* msg;
        std::size_t size;
    };

  public:
    explicit AsyncLogger(Logger& logger);
    ~AsyncLogger() override;

    // Copy.
    AsyncLogger(const AsyncLogger&) = delete;
    AsyncLogger& operator=(const AsyncLogger&) = delete;

    // Move.
    AsyncLogger(AsyncLogger&&) = delete;
    AsyncLogger& operator=(AsyncLogger&&) = delete;

    /// The run function waits for a log entry and then writes that log entry to the underlying
    /// logger. Returns false if the Logger was closed.
    bool run();

    /// Interrupt and exit any inprogress call to run().
    void stop();

  private:
    void write_all_messages();
    void do_write_log(WallTime ts, LogLevel level, int tid, LogMsgPtr&& msg,
                      std::size_t size) noexcept override;

    Logger& logger_;
    boost::lockfree::queue<Task, boost::lockfree::fixed_sized<false>> tq_{512};
    std::atomic<bool> stop_{false};
};

/// ScopedLogLevel provides a convenient RAII-style utility for setting the log-level for the
/// duration of a scoped block.
class TOOLBOX_API ScopedLogLevel {
  public:
    explicit ScopedLogLevel(LogLevel level) noexcept
    : prev_{set_log_level(level)}
    {
    }
    ~ScopedLogLevel() { set_log_level(prev_); }

    // Copy.
    ScopedLogLevel(const ScopedLogLevel&) = delete;
    ScopedLogLevel& operator=(const ScopedLogLevel&) = delete;

    // Move.
    ScopedLogLevel(ScopedLogLevel&&) = delete;
    ScopedLogLevel& operator=(ScopedLogLevel&&) = delete;

  private:
    const LogLevel prev_;
};

/// ScopedLogger provides a convenient RAII-style utility for setting the backend logger for the
/// duration of a scoped block.
class TOOLBOX_API ScopedLogger {
  public:
    explicit ScopedLogger(Logger& logger) noexcept
    : prev_{set_logger(logger)}
    {
    }
    ~ScopedLogger() { set_logger(prev_); }

    // Copy.
    ScopedLogger(const ScopedLogger&) = delete;
    ScopedLogger& operator=(const ScopedLogger&) = delete;

    // Move.
    ScopedLogger(ScopedLogger&&) = delete;
    ScopedLogger& operator=(ScopedLogger&&) = delete;

  private:
    Logger& prev_;
};

template <typename StreamT>
    requires Streamable<StreamT>
StreamT& operator<<(StreamT& os, LogLevel level)
{
    os << log_label(level);
    return os;
}

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_LOGGER_HPP
