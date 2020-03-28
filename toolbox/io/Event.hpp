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

#ifndef TOOLBOX_IO_EVENT_HPP
#define TOOLBOX_IO_EVENT_HPP

#include <cstdint>
#include <type_traits>
#include <utility>

#include <sys/epoll.h>

namespace toolbox {
inline namespace io {

struct MsgEvent {
    int type;
    char data[1524];
};
static_assert(std::is_pod_v<MsgEvent>);
static_assert(sizeof(MsgEvent) + sizeof(std::int64_t) == 1536);

template <typename DataT>
void emplace_event(MsgEvent& ev, int type) noexcept
{
    static_assert(alignof(DataT) <= 8);
    static_assert(std::is_nothrow_default_constructible_v<DataT>);
    static_assert(std::is_trivially_copyable_v<DataT>);
    ev.type = type;
    ::new (ev.data) DataT{};
}

template <typename DataT, typename... ArgsT>
void emplace_event(MsgEvent& ev, int type, ArgsT&&... args) noexcept
{
    static_assert(alignof(DataT) <= 8);
    static_assert(std::is_nothrow_constructible_v<DataT, ArgsT...>);
    static_assert(std::is_trivially_copyable_v<DataT>);
    ev.type = type;
    ::new (ev.data) DataT{std::forward<ArgsT>(args)...};
}

template <typename DataT>
const DataT& data(const MsgEvent& ev) noexcept
{
    return *reinterpret_cast<const DataT*>(ev.data);
}

template <typename DataT>
DataT& data(MsgEvent& ev) noexcept
{
    return *reinterpret_cast<DataT*>(ev.data);
}

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_EVENT_HPP
