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

#ifndef TOOLBOX_IPC_MSG_HPP
#define TOOLBOX_IPC_MSG_HPP

#include <toolbox/ipc/MpmcQueue.hpp>

namespace toolbox {
inline namespace ipc {

using MsgData = char[MaxMsgSize];
using MsgQueue = MpmcQueue<MsgData>;

inline void new_msg_queue(FileHandle& fh, std::size_t capacity)
{
    new_mpmc_queue<MsgData>(fh, capacity);
}

inline void new_msg_queue(FileHandle&& fh, std::size_t capacity)
{
    new_mpmc_queue<MsgData>(fh, capacity);
}

} // namespace ipc
} // namespace toolbox

#endif // TOOLBOX_IPC_MSG_HPP
