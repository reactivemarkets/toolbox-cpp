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

#include "Time.hpp"

namespace toolbox {
inline namespace sys {
using namespace std;

TOOLBOX_WEAK Nanos get_time(clockid_t clock_id) noexcept;

NanoTime get_time(clockid_t clock_id) noexcept
{
    timespec ts;
    clock_gettime(clock_id, &ts);
    // Long-long literal is required here for ARM32.
    return NanoTime{ts.tv_sec * 1'000'000'000LL + ts.tv_nsec};
}

ostream& operator<<(ostream& os, MonoTime t)
{
    return os << ns_since_epoch(t);
}

ostream& operator<<(ostream& os, WallTime t)
{
    return os << ns_since_epoch(t);
}

thread_local CyclTime::Time CyclTime::time_;

} // namespace sys
} // namespace toolbox
