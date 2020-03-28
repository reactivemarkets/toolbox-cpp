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

#ifndef TOOLBOX_UTIL_STRUCT_HPP
#define TOOLBOX_UTIL_STRUCT_HPP

#include <cstdint>

// The Struct concept is essentially a tag-based tuple that was inspired by:
// 1. Bronek Kozicki and his open souce projects (https://github.com/Bronek)
// 2. std::tuple<>
// 3. http://blogs.microsoft.co.il/sasha/2015/01/12/implementing-tuple-part-1/

namespace toolbox {
inline namespace util {
namespace detail {

template <typename TagT, typename ValueT>
struct Member {
    constexpr explicit Member(const ValueT& value)
    : value{value}
    {
    }
    ValueT value;
};

template <typename...>
struct Struct;

template <>
struct Struct<> {

    enum : std::size_t { Size = 0 };

    template <typename TagT, typename ValueT>
    constexpr auto extend(const ValueT& head) const
    {
        return Struct<Member<TagT, ValueT>>{head};
    }

    constexpr const auto& slice(...) const noexcept { return *this; }
    constexpr auto& slice(...) noexcept { return *this; }
};

template <typename TagT, typename ValueT, typename... TagsT, typename... ValuesT>
struct Struct<Member<TagT, ValueT>, Member<TagsT, ValuesT>...> : Struct<Member<TagsT, ValuesT>...> {

    using Tail = Struct<Member<TagsT, ValuesT>...>;
    enum : std::size_t { Size = 1 + Tail::Size };

    constexpr explicit Struct(const ValueT& head)
    : head{head}
    {
    }
    constexpr Struct(const ValueT& head, const Tail& tail)
    : Tail{tail}
    , head{head}
    {
    }
    template <typename TagU, typename ValueU>
    constexpr auto extend(const ValueU& head) const
    {
        return Struct<Member<TagU, ValueU>, Member<TagT, ValueT>, //
                      Member<TagsT, ValuesT>...>{head, *this};
    }

    using Tail::slice;
    constexpr const auto& slice(TagT) const noexcept { return *this; };
    constexpr auto& slice(TagT) noexcept { return *this; };

    ValueT head;
};

} // namespace detail

constexpr auto Struct = detail::Struct<>{};

template <typename... TagsT, typename... ValuesT>
constexpr bool empty(const detail::Struct<detail::Member<TagsT, ValuesT>...>& s)
{
    return s.Size == 0;
}

template <typename... TagsT, typename... ValuesT>
constexpr std::size_t size(const detail::Struct<detail::Member<TagsT, ValuesT>...>& s)
{
    return s.Size;
}

template <typename TagT, typename... TagsT, typename... ValuesT>
constexpr bool has(const detail::Struct<detail::Member<TagsT, ValuesT>...>& s, TagT tag = {})
{
    return s.slice(tag).Size > 0;
}

template <typename TagT, typename... TagsT, typename... ValuesT>
constexpr const auto& get(const detail::Struct<detail::Member<TagsT, ValuesT>...>& s, TagT tag = {})
{
    return s.slice(tag).head;
}

template <typename TagT, typename... TagsT, typename... ValuesT>
constexpr auto& get(detail::Struct<detail::Member<TagsT, ValuesT>...>& s, TagT tag = {})
{
    return s.slice(tag).head;
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STRUCT_HPP
