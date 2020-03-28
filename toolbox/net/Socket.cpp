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

#include "Socket.hpp"

#include <toolbox/util/String.hpp>

#include <sys/un.h>

namespace toolbox {
inline namespace net {
using namespace std;
namespace {
void free_unix_addrinfo(addrinfo* ai)
{
    if (ai) {
        delete reinterpret_cast<sockaddr_un*>(ai->ai_addr);
        delete ai;
    }
}
} // namespace
AddrInfoPtr get_unix_addrinfo(string_view path, int type)
{
    auto sun = make_unique<sockaddr_un>();
    sun->sun_family = AF_UNIX;
    const socklen_t path_len = pstrcpy<'\0'>(sun->sun_path, path);
    // Check that path has not exceeded max length.
    if (path_len == sizeof(sun->sun_path)) {
        throw invalid_argument{"invalid unix domain address"};
    }

    AddrInfoPtr ai{new addrinfo{}, free_unix_addrinfo};
    ai->ai_family = AF_UNIX;
    ai->ai_socktype = type;
    // Size includes null terminator. See unix(7).
    ai->ai_addrlen = offsetof(sockaddr_un, sun_path) + path_len + 1;
    ai->ai_addr = reinterpret_cast<sockaddr*>(sun.release());
    return ai;
}
} // namespace net
} // namespace toolbox
