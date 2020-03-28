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

#ifndef TOOLBOX_IO_RUNNER_HPP
#define TOOLBOX_IO_RUNNER_HPP

#include <toolbox/sys/Thread.hpp>

#include <atomic>
#include <thread>

namespace toolbox {
inline namespace io {
class Reactor;

class TOOLBOX_API ReactorRunner {
  public:
    ReactorRunner(Reactor& r, ThreadConfig config);
    ~ReactorRunner();

    // Copy.
    ReactorRunner(const ReactorRunner&) = delete;
    ReactorRunner& operator=(const ReactorRunner&) = delete;

    // Move.
    ReactorRunner(ReactorRunner&&) = delete;
    ReactorRunner& operator=(ReactorRunner&&) = delete;

  private:
    Reactor& reactor_;
    std::atomic<bool> stop_{false};
    std::thread thread_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_RUNNER_HPP
