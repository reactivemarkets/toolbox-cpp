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

#ifndef TOOLBOX_SYS_LOG_HPP
#define TOOLBOX_SYS_LOG_HPP

#include <toolbox/sys/Limits.hpp>
#include <toolbox/util/Stream.hpp>

namespace toolbox {
inline namespace sys {

/// Logger callback function.
using Logger = void (*)(int, std::string_view);

/// Return log label for given log level.
TOOLBOX_API const char* log_label(int level) noexcept;

/// Return current log level.
TOOLBOX_API int get_log_level() noexcept;

/// Return true if level is less than or equal to current log level.
inline bool is_log_level(int level) noexcept
{
    return level <= get_log_level();
}

/// Set log level globally for all threads.
TOOLBOX_API int set_log_level(int level) noexcept;

/// Return current logger.
TOOLBOX_API Logger get_logger() noexcept;

/// Set logger globally for all threads.
TOOLBOX_API Logger set_logger(Logger logger) noexcept;

/// Unconditionally write log message to the logger. Specifically, this function does not check that
/// level is allowed by the current log level; users are expected to call is_log_level() first,
/// before formatting the log message.
TOOLBOX_API void write_log(int level, std::string_view msg) noexcept;

/// Null logger. This logger does nothing and is effectively /dev/null.
TOOLBOX_API void null_logger(int level, std::string_view msg) noexcept;

/// Standard logger. This logger writes to stdout if the log level is greater than LogWarn, and
/// stdout otherwise.
TOOLBOX_API void std_logger(int level, std::string_view msg) noexcept;

/// System logger. This logger calls syslog().
TOOLBOX_API void sys_logger(int level, std::string_view msg) noexcept;

/// Logger callback function.
using LogMsg = StaticStream<MaxMsgSize>;

/// Thread-local log message. This thread-local instance of StaticStream can be used to format log
/// messages before writing to the log. Note that the StaticStream is reset each time this function
/// is called.
TOOLBOX_API LogMsg& log_msg() noexcept;

// Inspired by techniques developed by Rodrigo Fernandes.
class Log {
    template <typename ValueT>
    friend Log& operator<<(Log& log, ValueT&& val)
    {
        log.msg_ << std::forward<ValueT>(val);
        return log;
    }
    template <typename ValueT>
    friend Log& operator<<(Log&& log, ValueT&& val)
    {
        log.msg_ << std::forward<ValueT>(val);
        return log;
    }

  public:
    enum : int {
        /// Critical.
        Crit,
        /// Error.
        Error,
        /// Warning.
        Warning,
        /// Notice.
        Notice,
        /// Information.
        Info,
        /// Debug.
        Debug
    };

    explicit Log(int level) noexcept
    : level_{level}
    , msg_{log_msg()}
    {
    }
    ~Log() { write_log(level_, msg_); }

    // Copy.
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    // Move.
    Log(Log&&) = delete;
    Log& operator=(Log&&) = delete;

    constexpr explicit operator bool() const { return true; }

  private:
    const int level_;
    LogMsg& msg_;
};

} // namespace sys
} // namespace toolbox

// clang-format off
#define TOOLBOX_LOG(LEVEL) toolbox::is_log_level(LEVEL) && toolbox::Log{LEVEL}

#define TOOLBOX_CRIT TOOLBOX_LOG(toolbox::Log::Crit)
#define TOOLBOX_ERROR TOOLBOX_LOG(toolbox::Log::Error)
#define TOOLBOX_WARNING TOOLBOX_LOG(toolbox::Log::Warning)
#define TOOLBOX_NOTICE TOOLBOX_LOG(toolbox::Log::Notice)
#define TOOLBOX_INFO TOOLBOX_LOG(toolbox::Log::Info)

#if TOOLBOX_BUILD_DEBUG
#define TOOLBOX_DEBUG TOOLBOX_LOG(toolbox::Log::Debug)
#else
#define TOOLBOX_DEBUG false && toolbox::Log{toolbox::Log::Debug}
#endif
// clang-format on

#endif // TOOLBOX_SYS_LOG_HPP
