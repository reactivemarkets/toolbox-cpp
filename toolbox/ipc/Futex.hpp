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

#ifndef TOOLBOX_IPC_FUTEX_HPP
#define TOOLBOX_IPC_FUTEX_HPP

#include <toolbox/sys/Error.hpp>

#include <limits>

#include <unistd.h>

#include <linux/futex.h>
#include <sys/syscall.h>
#include <sys/time.h>

namespace toolbox {
inline namespace ipc {
namespace detail {
inline int futex(int& uaddr, int futex_op, int val, const timespec* timeout = nullptr,
                 int* uaddr2 = nullptr, int val3 = 0) noexcept
{
    return syscall(SYS_futex, &uaddr, futex_op, val, timeout, uaddr, val3);
}
} // namespace detail

/// This operation wakes \p n waiters at most that are waiting (e.g., inside `FUTEX_WAIT`) on the
/// futex word at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup(int& uaddr, int n, std::error_code& ec) noexcept
{
    const auto ret = detail::futex(uaddr, FUTEX_WAKE, n);
    if (ret < 0) {
        ec = make_sys_error(errno);
    }
    return ret;
}

/// This operation wakes \p n waiters at most that are waiting (e.g., inside `FUTEX_WAIT`) on the
/// futex word at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup(int& uaddr, int n)
{
    const auto ret = detail::futex(uaddr, FUTEX_WAKE, n);
    if (ret < 0) {
        throw std::system_error{make_sys_error(errno), "futex"};
    }
    return ret;
}

/// This operation wakes all waiters that are waiting (e.g., inside `FUTEX_WAIT`) on the futex word
/// at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup_all(int& uaddr, std::error_code& ec) noexcept
{
    return futex_wakeup(uaddr, std::numeric_limits<int>::max(), ec);
}

/// This operation wakes all waiters that are waiting (e.g., inside `FUTEX_WAIT`) on the futex word
/// at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup_all(int& uaddr)
{
    return futex_wakeup(uaddr, std::numeric_limits<int>::max());
}

/// This operation wakes at most a single waiter that is waiting (e.g., inside `FUTEX_WAIT`) on the
/// futex word at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup_one(int& uaddr, std::error_code& ec) noexcept
{
    return futex_wakeup(uaddr, 1, ec);
}

/// This operation wakes at most a single waiter that is waiting (e.g., inside `FUTEX_WAIT`) on the
/// futex word at the address \p uaddr.
///
/// \return the number of waiters that were woken up.
inline int futex_wakeup_one(int& uaddr)
{
    return futex_wakeup(uaddr, 1);
}

/// This operation tests that the value at the futex word pointed to by the address \p uaddr still
/// contains the \p expected value, and if so, then sleeps waiting for a `FUTEX_WAKE` operation on
/// the futex word. If the futex value does not match \p expected, then the call fails immediately
/// with the error `EAGAIN`.
inline void futex_wait(int& uaddr, int expected, std::error_code& ec) noexcept
{
    if (detail::futex(uaddr, FUTEX_WAIT, expected) < 0) {
        ec = make_sys_error(errno);
    }
}

/// This operation tests that the value at the futex word pointed to by the address \p uaddr still
/// contains the \p expected value, and if so, then sleeps waiting for a `FUTEX_WAKE` operation on
/// the futex word.
///
/// \return false if the futex value does not match \p expected.
inline bool futex_wait(int& uaddr, int expected)
{
    if (detail::futex(uaddr, FUTEX_WAIT, expected) < 0) {
        if (errno == EAGAIN) {
            return false;
        }
        throw std::system_error{make_sys_error(errno), "futex"};
    }
    return true;
}

} // namespace ipc
} // namespace toolbox

#endif // TOOLBOX_IPC_FUTEX_HPP
