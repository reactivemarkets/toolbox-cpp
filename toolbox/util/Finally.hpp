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

#ifndef TOOLBOX_UTIL_FINALLY_HPP
#define TOOLBOX_UTIL_FINALLY_HPP

#include <utility>

namespace toolbox {
inline namespace util {

template <typename FnT>
class Finally {
    static_assert(std::is_nothrow_invocable_v<FnT>);

  public:
    explicit Finally(FnT fn) noexcept
    : fn_{std::move(fn)}
    {
    }
    ~Finally() { fn_(); }
    // Copy.
    Finally(const Finally&) = delete;
    Finally& operator=(const Finally&) = delete;

    // Move.
    Finally(Finally&&) = default;
    Finally& operator=(Finally&&) = delete;

  private:
    FnT fn_;
};

template <typename FnT>
auto make_finally(FnT fn) noexcept
{
    return Finally<FnT>{std::move(fn)};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_FINALLY_HPP
