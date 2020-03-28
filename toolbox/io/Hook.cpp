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

#include "Hook.hpp"

#include <toolbox/sys/Log.hpp>

namespace toolbox {
inline namespace io {

void dispatch(CyclTime now, const HookList& l) noexcept
{
    auto it = l.begin();
    while (it != l.end()) {
        // Increment iterator before calling each hook, so that hooks can safely unhook themselves.
        const auto prev = it++;
        try {
            prev->slot(now);
        } catch (const std::exception& e) {
            TOOLBOX_ERROR << "error handling hook: " << e.what();
        }
    }
}

} // namespace io
} // namespace toolbox
