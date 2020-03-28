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

#ifndef TOOLBOX_NET_IOSOCK_HPP
#define TOOLBOX_NET_IOSOCK_HPP

#include <toolbox/net/Socket.hpp>

namespace toolbox {
inline namespace net {

/// Socket with IO operations. I.e. not a passive listener. All state is in base class, so object can
/// be sliced.
struct IoSock : Sock {
    using Sock::Sock;

    IoSock() noexcept = default;

    void shutdown(int how) { return os::shutdown(get(), how); }

    ssize_t recv(void* buf, std::size_t len, int flags, std::error_code& ec) noexcept
    {
        return os::recv(get(), buf, len, flags, ec);
    }
    std::size_t recv(void* buf, std::size_t len, int flags)
    {
        return os::recv(get(), buf, len, flags);
    }

    ssize_t recv(MutableBuffer buf, int flags, std::error_code& ec) noexcept
    {
        return os::recv(get(), buf, flags, ec);
    }
    std::size_t recv(MutableBuffer buf, int flags) { return os::recv(get(), buf, flags); }

    ssize_t send(const void* buf, std::size_t len, int flags, std::error_code& ec) noexcept
    {
        return os::send(get(), buf, len, flags, ec);
    }
    std::size_t send(const void* buf, std::size_t len, int flags)
    {
        return os::send(get(), buf, len, flags);
    }

    ssize_t send(ConstBuffer buf, int flags, std::error_code& ec) noexcept
    {
        return os::send(get(), buf, flags, ec);
    }
    std::size_t send(ConstBuffer buf, int flags) { return os::send(get(), buf, flags); }
};

template <typename ProtocolT>
inline std::pair<IoSock, IoSock> socketpair(ProtocolT protocol)
{
    auto socks = os::socketpair(protocol);
    return {IoSock{std::move(socks.first), protocol.family()},
            IoSock{std::move(socks.second), protocol.family()}};
}

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_IOSOCK_HPP
