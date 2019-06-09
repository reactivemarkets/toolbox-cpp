// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
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

#ifndef TOOLBOX_IO_INTERRUPTIBLE_HPP
#define TOOLBOX_IO_INTERRUPTIBLE_HPP

#include <toolbox/Config.h>

namespace toolbox {
inline namespace io {

class TOOLBOX_API Interruptible {
  public:
    Interruptible() noexcept = default;
    virtual ~Interruptible();

    // Copy.
    Interruptible(const Interruptible&) = delete;
    Interruptible& operator=(const Interruptible&) = delete;

    // Move.
    Interruptible(Interruptible&&) = delete;
    Interruptible& operator=(Interruptible&&) = delete;

    void interrupt() noexcept { do_interrupt(); }

  protected:
    virtual void do_interrupt() noexcept = 0;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_INTERRUPTIBLE_HPP
