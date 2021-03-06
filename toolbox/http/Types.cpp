// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Types.hpp"

namespace toolbox {
inline namespace http {

const char* enum_string(Status status) noexcept
{
    switch (static_cast<int>(status)) {
#define XX(num, name, string)                                                                      \
    case num:                                                                                      \
        return #string;
        HTTP_STATUS_MAP(XX)
#undef XX
    }
    std::terminate();
}

} // namespace http
} // namespace toolbox
