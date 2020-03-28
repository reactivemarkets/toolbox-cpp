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

#ifndef TOOLBOX_UTIL_ALARM_HPP
#define TOOLBOX_UTIL_ALARM_HPP

#include <thread>

namespace toolbox {
inline namespace util {

class Alarm {
  public:
    template <typename RepT, typename PeriodT, typename FnT>
    explicit Alarm(std::chrono::duration<RepT, PeriodT> d, FnT fn) noexcept
    : thread_{run<RepT, PeriodT, FnT>, d, std::move(fn)}
    {
    }
    ~Alarm() { thread_.join(); }

    // Copy.
    Alarm(const Alarm&) = delete;
    Alarm& operator=(const Alarm&) = delete;

    // Move.
    Alarm(Alarm&&) = delete;
    Alarm& operator=(Alarm&&) = delete;

  private:
    template <typename RepT, typename PeriodT, typename FnT>
    static void run(std::chrono::duration<RepT, PeriodT> d, FnT fn)
    {
        std::this_thread::sleep_for(d);
        fn();
    }
    std::thread thread_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ALARM_HPP
