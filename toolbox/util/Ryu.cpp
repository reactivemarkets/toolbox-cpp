// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
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

#include "Ryu.hpp"

#include <toolbox/contrib/ryu/d2s.c>

namespace toolbox {
inline namespace util {

std::size_t d2s_buffered_n(double f, char* result) noexcept
{
    std::size_t n = ::d2s_buffered_n(f, result);
    // Trim superfluous "E0" suffix.
    if (n > 2 && result[n - 2] == 'E' && result[n - 1] == '0') {
        n -= 2;
    }
    return n;
}

} // namespace util
} // namespace toolbox
