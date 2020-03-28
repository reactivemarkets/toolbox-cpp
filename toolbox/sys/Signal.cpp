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

#include "Signal.hpp"

#include "Error.hpp"

#include <toolbox/util/Finally.hpp>

namespace toolbox {
inline namespace sys {

SigWait::SigWait(std::initializer_list<int> mask)
{
    sigemptyset(&new_mask_);
    for (auto sig : mask) {
        sigaddset(&new_mask_, sig);
    }

    const auto err = pthread_sigmask(SIG_BLOCK, &new_mask_, &old_mask_);
    if (err != 0) {
        throw std::system_error{make_sys_error(err), "pthread_sigmask"};
    }
}

SigWait::~SigWait()
{
    // Restore original signal mask.
    pthread_sigmask(SIG_SETMASK, &old_mask_, nullptr);
}

int SigWait::operator()() const
{
    const auto finally = make_finally([&]() noexcept { CyclTime::now(); });

    siginfo_t info;
    if (sigwaitinfo(&new_mask_, &info) < 0) {
        throw std::system_error{make_sys_error(errno), "sigwaitinfo"};
    }
    return info.si_signo;
}

int SigWait::operator()(Duration timeout) const
{
    const auto finally = make_finally([&]() noexcept { CyclTime::now(); });

    siginfo_t info;
    const auto ts = to_timespec(timeout);
    if (sigtimedwait(&new_mask_, &info, &ts) < 0) {
        if (errno == EAGAIN) {
            // Timeout.
            return 0;
        }
        throw std::system_error{make_sys_error(errno), "sigtimedwait"};
    }
    return info.si_signo;
}

void sig_block_all()
{
    sigset_t ss;
    sigfillset(&ss);
    const auto err = pthread_sigmask(SIG_SETMASK, &ss, nullptr);
    if (err != 0) {
        throw std::system_error{make_sys_error(err), "pthread_sigmask"};
    }
}

} // namespace sys
} // namespace toolbox
