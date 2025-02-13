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

#ifndef TOOLBOX_SYS_LOG_HPP
#define TOOLBOX_SYS_LOG_HPP

#include <toolbox/sys/Logger.hpp>

#include <toolbox/util/Stream.hpp>

namespace toolbox {
inline namespace sys {

/// Logger callback function.
using LogStream = util::OStream<MaxLogLine>;

/// Thread-local log stream. This thread-local instance of OStream can be used to format log
/// messages before writing them the log.
TOOLBOX_API LogStream& log_stream() noexcept;

// Inspired by techniques developed by Rodrigo Fernandes.
class Log {
    template <typename ValueT>
    friend Log& operator<<(Log& log, ValueT&& val)
    {
        log.os_ << std::forward<ValueT>(val);
        return log;
    }

  public:
    explicit Log(WallTime ts, LogLevel level) noexcept
    : ts_{ts}
    , level_{level}
    , os_{log_stream()}
    {
        os_.set_storage(os_.make_storage());
    }
    ~Log()
    {
        const auto size{os_.size()};
        write_log(ts_, level_, os_.release_storage(), size);
    }

    // Copy.
    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    // Move.
    Log(Log&&) = delete;
    Log& operator=(Log&&) = delete;

    constexpr explicit operator bool() const { return true; }
    /// Function operator provided for writing unformatted data to the log.
    Log& operator()(const char* data, std::streamsize size)
    {
        os_.write(data, size);
        return *this;
    }
    /// Function operator provided for rvalue to lvalue conversion.
    Log& operator()() noexcept { return *this; }

  private:
    const WallTime ts_;
    const LogLevel level_;
    LogStream& os_;
};

} // namespace sys
} // namespace toolbox

// clang-format off
#define TOOLBOX_LOG(LEVEL) \
toolbox::is_log_level(LEVEL) && toolbox::Log{toolbox::WallClock::now(), LEVEL}()

#define TOOLBOX_CRIT TOOLBOX_LOG(toolbox::LogLevel::Crit)
#define TOOLBOX_ERROR TOOLBOX_LOG(toolbox::LogLevel::Error)
#define TOOLBOX_WARN TOOLBOX_LOG(toolbox::LogLevel::Warn)
#define TOOLBOX_METRIC TOOLBOX_LOG(toolbox::LogLevel::Metric)
#define TOOLBOX_NOTICE TOOLBOX_LOG(toolbox::LogLevel::Notice)
#define TOOLBOX_INFO TOOLBOX_LOG(toolbox::LogLevel::Info)

#if TOOLBOX_BUILD_DEBUG
#define TOOLBOX_DEBUG TOOLBOX_LOG(toolbox::LogLevel::Debug)
#else
#define TOOLBOX_DEBUG false && toolbox::Log{WallClock::now(), toolbox::LogLevel::Debug}()
#endif
// clang-format on

#endif // TOOLBOX_SYS_LOG_HPP
