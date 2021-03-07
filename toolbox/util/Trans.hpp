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

#ifndef TOOLBOX_UTIL_TRANS_HPP
#define TOOLBOX_UTIL_TRANS_HPP

#include <type_traits>

namespace toolbox {
inline namespace util {

template <typename TargetT>
struct TransTraits {
    static void start(TargetT& target) { target.start(); }
    static void commit(TargetT& target) { target.commit(); }
    static void rollback(TargetT& target) noexcept
    {
        static_assert(std::is_nothrow_invocable_v<decltype(&TargetT::rollback), TargetT>);
        target.rollback();
    }
};

template <typename TargetT, typename TraitsT = TransTraits<TargetT>>
class BasicTrans {
  public:
    explicit BasicTrans(TargetT& target)
    : target_{target}
    {
        TraitsT::start(target_);
    }
    ~BasicTrans()
    {
        if (!done_) {
            TraitsT::rollback(target_);
        }
    }

    // Copy.
    BasicTrans(const BasicTrans&) = delete;
    BasicTrans& operator=(const BasicTrans&) = delete;

    // Move.
    BasicTrans(BasicTrans&&) = delete;
    BasicTrans& operator=(BasicTrans&&) = delete;

    void commit()
    {
        TraitsT::commit(target_);
        done_ = true;
    }

  private:
    TargetT& target_;
    bool done_{false};
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TRANS_HPP
