// The Reactive C++ Toolbox.
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

#include <toolbox/sys/Log.hpp>

#include <toolbox/sys/Runner.hpp>

#include <toolbox/io/File.hpp>

#include <toolbox/bm.hpp>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {
namespace noformat {
// Specific Log operator<< to allow non formatted writing.
Log& operator<<(Log& log, std::string_view str)
{
    return log(str.data(), str.size());
}

class FileLogger final : public Logger {
  public:
    FileLogger()
    : fh_{os::open("/dev/null", O_RDWR)}
    {
    }

  private:
    void do_write_log(WallTime ts, LogLevel level, LogMsgPtr&& msg, size_t size) noexcept override
    {
        os::write(*fh_, msg.get(), size);
    }
    io::FileHandle fh_;
};

} // namespace noformat

TOOLBOX_BENCHMARK(log_formatted)
{
    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{null_logger()};
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            TOOLBOX_LOG(LogLevel::Info) << "BenchmarkString"sv;
        }
    }
}

TOOLBOX_BENCHMARK(log_unformatted)
{
    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{null_logger()};
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            using namespace noformat;
            TOOLBOX_LOG(LogLevel::Info) << "BenchmarkString"sv;
        }
    }
}

TOOLBOX_BENCHMARK(call_logger)
{
    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{null_logger()};
    while (ctx) {
        for (auto _ : ctx.range(1000)) {
            write_log(WallTime{}, LogLevel::Info, LogMsgPtr{}, 0);
        }
    }
}

TOOLBOX_BENCHMARK(call_async_logger)
{
    using namespace noformat;

    AsyncLogger al{null_logger()};
    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger asl{al};
    while (ctx) {
        for (auto _ : ctx.range(10)) {
            TOOLBOX_INFO << "foobar: "sv << 101;
        }
        // Drain items.
        for (int i{0}; i < 10; ++i) {
            if (!al.run()) {
                break;
            }
        }
    }
}

TOOLBOX_BENCHMARK(sync_null_logger)
{
    using namespace noformat;

    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{null_logger()};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            TOOLBOX_INFO << "foobar: "sv << 101;
        }
    }
}

TOOLBOX_BENCHMARK(sync_file_logger)
{
    using namespace noformat;

    FileLogger fl;
    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{fl};
    while (ctx) {
        for (auto _ : ctx.range(50)) {
            TOOLBOX_INFO << "foobar: "sv << 101;
        }
    }
}

TOOLBOX_BENCHMARK(async_null_logger)
{
    using namespace noformat;

    ScopedLogLevel sll{LogLevel::Info};
    ScopedLogger sl{null_logger()};

    AsyncLogger al{get_logger()};
    Runner alr{al, "logger"s};
    ScopedLogger asl{al};
    while (ctx) {
        for (auto _ : ctx.range(50)) {
            TOOLBOX_INFO << "foobar: "sv << 101;
        }
        // Log 100 items then backoff to avoid flooding the queue.
        this_thread::sleep_for(1ms);
    }
}

} // namespace
