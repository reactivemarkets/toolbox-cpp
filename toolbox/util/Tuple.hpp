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

#ifndef TOOLBOX_UTIL_TUPLE_HPP
#define TOOLBOX_UTIL_TUPLE_HPP

#include <tuple>
#include <utility>

namespace toolbox {
inline namespace util {

template <std::size_t IndexN, typename TupleT, typename FnT>
void tuple_apply(const TupleT& tpl, const FnT& f)
{
    const auto& v = std::get<IndexN>(tpl);
    f(v);
}

/// Calling fn over each tuple element.
/// N.B the limitation is function object must be an template lambda or function, accept
/// all the types in the tuple.
template <typename TupleT, typename FnT, std::size_t IndexN = 0>
void tuple_for_each(const TupleT& tpl, const FnT& f)
{
    constexpr auto tuple_size = std::tuple_size_v<TupleT>;
    if constexpr (IndexN < tuple_size) {
        tuple_apply<IndexN>(tpl, f);
        tuple_for_each<TupleT, FnT, IndexN + 1>(tpl, f);
    }
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TUPLE_HPP
