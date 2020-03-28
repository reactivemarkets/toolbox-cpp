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

#ifndef TOOLBOX_IO_NOTIFIABLE_HPP
#define TOOLBOX_IO_NOTIFIABLE_HPP

#include <toolbox/Config.h>

namespace toolbox {
inline namespace io {

/// The Notifiable is implemented by types that may be woken-up, interrupted or otherwise notified
/// of an asynchronous event.
class TOOLBOX_API Notifiable {
  public:
    Notifiable() noexcept = default;
    virtual ~Notifiable();

    // Copy.
    Notifiable(const Notifiable&) noexcept = default;
    Notifiable& operator=(const Notifiable&) noexcept = default;

    // Move.
    Notifiable(Notifiable&&) noexcept = default;
    Notifiable& operator=(Notifiable&&) noexcept = default;

    void notify() noexcept { do_notify(); }

  protected:
    virtual void do_notify() noexcept = 0;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_NOTIFIABLE_HPP
