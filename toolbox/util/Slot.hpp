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

#ifndef TOOLBOX_UTIL_SLOT_HPP
#define TOOLBOX_UTIL_SLOT_HPP

#include <toolbox/util/Traits.hpp>

#include <utility>

namespace toolbox {
inline namespace util {

template <typename... ArgsT>
class BasicSlot {
  public:
    friend constexpr bool operator==(BasicSlot lhs, BasicSlot rhs) noexcept
    {
        return lhs.obj_ == rhs.obj_ && lhs.fn_ == rhs.fn_;
    }
    friend constexpr bool operator!=(BasicSlot lhs, BasicSlot rhs) noexcept
    {
        return !(lhs == rhs);
    }
    constexpr BasicSlot(std::nullptr_t = nullptr) noexcept {}
    ~BasicSlot() = default;

    // Copy.
    constexpr BasicSlot(const BasicSlot&) noexcept = default;
    constexpr BasicSlot& operator=(const BasicSlot&) noexcept = default;

    // Move.
    constexpr BasicSlot(BasicSlot&&) noexcept = default;
    constexpr BasicSlot& operator=(BasicSlot&&) noexcept = default;

    void invoke(ArgsT... args) const { fn_(obj_, std::forward<ArgsT>(args)...); }
    void operator()(ArgsT... args) const { fn_(obj_, std::forward<ArgsT>(args)...); }
    constexpr bool empty() const noexcept { return fn_ == nullptr; }
    constexpr explicit operator bool() const noexcept { return fn_ != nullptr; }

    // Free function.
    template <void (*FnT)(ArgsT...)>
    constexpr auto& bind() noexcept
    {
        obj_ = nullptr;
        fn_ = [](void* obj, ArgsT... args) { FnT(std::forward<ArgsT>(args)...); };
        return *this;
    }
    // Lambda function.
    template <typename ClassT>
    constexpr auto& bind(ClassT* obj) noexcept
    {
        obj_ = obj;
        fn_ = [](void* obj, ArgsT... args) {
            (*static_cast<ClassT*>(obj))(std::forward<ArgsT>(args)...);
        };
        return *this;
    }
    // Member function.
    template <auto MemFnT, typename ClassT = typename FunctionTraits<decltype(MemFnT)>::ClassType>
    constexpr auto& bind(ClassT* obj) noexcept
    {
        obj_ = obj;
        fn_ = [](void* obj, ArgsT... args) {
            (static_cast<ClassT*>(obj)->*MemFnT)(std::forward<ArgsT>(args)...);
        };
        return *this;
    }
    void reset(std::nullptr_t = nullptr) noexcept
    {
        obj_ = nullptr;
        fn_ = nullptr;
    }

  private:
    void* obj_{nullptr};
    void (*fn_)(void*, ArgsT...){nullptr};
};

template <auto FnT>
constexpr auto bind() noexcept
{
    using Traits = FunctionTraits<decltype(FnT)>;
    using Slot = typename Traits::template Pack<BasicSlot>;
    return Slot{}.template bind<FnT>();
}

template <typename ClassT>
constexpr auto bind(ClassT* obj) noexcept
{
    using Traits = FunctionTraits<decltype(&ClassT::operator())>;
    using Slot = typename Traits::template Pack<BasicSlot>;
    return Slot{}.bind(obj);
}

template <auto MemFnT, typename ClassT = typename FunctionTraits<decltype(MemFnT)>::ClassType>
constexpr auto bind(ClassT* obj) noexcept
{
    using Traits = FunctionTraits<decltype(MemFnT)>;
    using Slot = typename Traits::template Pack<BasicSlot>;
    return Slot{}.template bind<MemFnT>(obj);
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_SLOT_HPP
