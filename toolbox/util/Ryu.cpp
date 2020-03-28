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

#include "Ryu.hpp"

#include <toolbox/contrib/ryu/d2fixed.c>
#include <toolbox/contrib/ryu/d2s.c>

namespace toolbox {
inline namespace util {
using namespace std;
namespace {
thread_local char ryu_buf_[MaxRyuFixedBuf + 1];
} // namespace

size_t dtos(char* dst, double d) noexcept
{
    size_t n = ::d2s_buffered_n(d, dst);
    // Trim superfluous "E0" suffix.
    if (n > 2 && dst[n - 2] == 'E' && dst[n - 1] == '0') {
        n -= 2;
    }
    return n;
}

string_view dtos(double d) noexcept
{
    const size_t n = dtos(ryu_buf_, d);
    return {ryu_buf_, n};
}

size_t dtofixed(char* dst, double d, int prec) noexcept
{
    assert(0 <= prec && prec <= MaxRyuPrec);
    size_t n = ::d2fixed_buffered_n(d, prec, dst);
    // Trim trailing zeros after the decimal place.
    while (prec-- > 0 && dst[n - 1] == '0') {
        --n;
    }
    if (dst[n - 1] == '.') {
        --n;
    }
    return n;
}

string_view dtofixed(double d, int prec) noexcept
{
    const size_t n = dtofixed(ryu_buf_, d, prec);
    return {ryu_buf_, n};
}

} // namespace util
} // namespace toolbox
