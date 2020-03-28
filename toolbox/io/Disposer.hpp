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

#ifndef TOOLBOX_IO_DISPOSER_HPP
#define TOOLBOX_IO_DISPOSER_HPP

#include <toolbox/sys/Time.hpp>
#include <toolbox/util/Finally.hpp>

namespace toolbox {
inline namespace io {

/// The BasicDisposer class template exposes a public dispose method that may be safely called
/// from callback functions while a lock is held.
template <typename DerivedT>
class BasicDisposer {
  public:
    BasicDisposer() noexcept = default;

    // Copy.
    BasicDisposer(const BasicDisposer&) = delete;
    BasicDisposer& operator=(const BasicDisposer&) = delete;

    // Move.
    BasicDisposer(BasicDisposer&&) = delete;
    BasicDisposer& operator=(BasicDisposer&&) = delete;

    /// The dispose method may be safely called from callback functions while a lock is held.
    void dispose(CyclTime now) noexcept
    {
        if (locks_ == 0) {
            // Prevent the object from being disposed a second time by holding a lock
            // indefinitely.
            locks_ = 1;
            static_cast<DerivedT*>(this)->dispose_now(now);
        } else {
            dispose_ = true;
        }
    }

  protected:
    ~BasicDisposer() = default;

    /// Returns true if the lock is held or the object has been disposed.
    bool is_locked() const noexcept { return locks_ > 0; }

    /// Returns a lock that prevents the disposer instance from being deleted while the lock is
    /// held.
    [[nodiscard]] auto lock_this(CyclTime now) noexcept
    {
        ++locks_;
        return make_finally([this, now]() noexcept {
            if (--locks_ == 0 && dispose_) {
                // Prevent the object from being disposed a second time by holding a lock
                // indefinitely.
                locks_ = 1;
                static_cast<DerivedT*>(this)->dispose_now(now);
            }
        });
    }

  private:
    short locks_{0};
    bool dispose_{false};
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_DISPOSER_HPP
