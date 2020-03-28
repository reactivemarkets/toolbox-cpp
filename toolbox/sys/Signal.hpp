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

#ifndef TOOLBOX_SYS_SIGNAL_HPP
#define TOOLBOX_SYS_SIGNAL_HPP

#include <toolbox/sys/Time.hpp>

#include <csignal>

namespace toolbox {
inline namespace sys {

class TOOLBOX_API SigWait {
  public:
    SigWait(std::initializer_list<int> mask = {SIGHUP, SIGINT, SIGUSR1, SIGUSR2, SIGTERM});
    ~SigWait();

    // Copy.
    SigWait(const SigWait&) = delete;
    SigWait& operator=(const SigWait&) = delete;

    // Move.
    SigWait(SigWait&&) = delete;
    SigWait& operator=(SigWait&&) = delete;

    int operator()() const;
    int operator()(Duration timeout) const;

    template <typename RepT, typename PeriodT>
    int operator()(std::chrono::duration<RepT, PeriodT> timeout) const
    {
        return this->operator()(std::chrono::duration_cast<Duration>(timeout));
    }

  private:
    sigset_t new_mask_, old_mask_;
};

/// Block all signals.
TOOLBOX_API void sig_block_all();

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_SIGNAL_HPP
