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

#ifndef TOOLBOX_SYS_RUNNER_HPP
#define TOOLBOX_SYS_RUNNER_HPP

#include <toolbox/sys/Log.hpp>
#include <toolbox/sys/Signal.hpp>
#include <toolbox/sys/Thread.hpp>

#include <thread>

namespace toolbox {
inline namespace sys {

namespace {
template <typename T>
// clang-format off
concept Runnable = requires (T r) {
    { r.run() } -> std::convertible_to<bool>;
    r.stop();
};
// clang-format on
} // namespace

template <typename RunnableT>
requires Runnable<RunnableT>
class Runner {
  public:
    Runner(RunnableT& r, ThreadConfig config)
    : r_{r}
    , thread_{run, std::ref(r), config}
    {
    }
    ~Runner()
    {
        r_.stop();
        thread_.join();
    }

    // Copy.
    Runner(const Runner&) = delete;
    Runner& operator=(const Runner&) = delete;

    // Move.
    Runner(Runner&&) = delete;
    Runner& operator=(Runner&&) = delete;

  private:
    static void run(RunnableT& r, ThreadConfig config)
    {
        sig_block_all();
        try {
            set_thread_attrs(config);
            TOOLBOX_NOTICE << "started " << config.name << " thread";
            // The run() function returns false when the runnable is stopped.
            while (r.run()) {
            }
        } catch (const std::exception& e) {
            TOOLBOX_CRIT << "exception on " << config.name << " thread: " << e.what();
            kill(getpid(), SIGTERM);
        }
        TOOLBOX_NOTICE << "stopping " << config.name << " thread";
    }

    RunnableT& r_;
    std::thread thread_;
};

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_RUNNER_HPP
