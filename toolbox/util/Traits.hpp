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

#ifndef TOOLBOX_UTIL_TRAITS_HPP
#define TOOLBOX_UTIL_TRAITS_HPP

#include <tuple>

namespace toolbox {
inline namespace util {

/// Default case for functors and lambdas.
template <typename TypeT>
struct FunctionTraits : FunctionTraits<decltype(&TypeT::operator())> {
};

/// Specialisation for free functions.
template <typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (*)(ArgsT...)> {

    using FunctionType = ReturnT (*)(ArgsT...);
    using ClassType = void;
    using ReturnType = ReturnT;

    enum { Arity = sizeof...(ArgsT) };

    template <std::size_t i>
    using ArgType = std::tuple_element_t<i, std::tuple<ArgsT...>>;

    // Apply parameter pack to template.
    template <template <typename...> typename TemplT>
    using Pack = TemplT<ArgsT...>;
};

/// Specialisation for noexcept free functions.
template <typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (*)(ArgsT...) noexcept> : FunctionTraits<ReturnT (*)(ArgsT...)> {
};

/// Specialisation for member functions.
template <typename ClassT, typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (ClassT::*)(ArgsT...)> : FunctionTraits<ReturnT (*)(ArgsT...)> {
    using FunctionType = ReturnT (ClassT::*)(ArgsT...);
    using ClassType = ClassT;
};

/// Specialisation for const member functions.
template <typename ClassT, typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (ClassT::*)(ArgsT...) const> : FunctionTraits<ReturnT (*)(ArgsT...)> {
    using FunctionType = ReturnT (ClassT::*)(ArgsT...) const;
    using ClassType = ClassT;
};

/// Specialisation for const noexcept member functions.
template <typename ClassT, typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (ClassT::*)(ArgsT...) const noexcept>
: FunctionTraits<ReturnT (*)(ArgsT...)> {
    using FunctionType = ReturnT (ClassT::*)(ArgsT...) const noexcept;
    using ClassType = ClassT;
};

/// Specialisation for noexcept member functions.
template <typename ClassT, typename ReturnT, typename... ArgsT>
struct FunctionTraits<ReturnT (ClassT::*)(ArgsT...) noexcept>
: FunctionTraits<ReturnT (*)(ArgsT...)> {
    using FunctionType = ReturnT (ClassT::*)(ArgsT...) noexcept;
    using ClassType = ClassT;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TRAITS_HPP
