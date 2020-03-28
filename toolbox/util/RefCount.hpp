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

#ifndef TOOLBOX_UTIL_REFCOUNT_HPP
#define TOOLBOX_UTIL_REFCOUNT_HPP

#include <toolbox/Config.h>

#include <boost/intrusive_ptr.hpp>

#include <atomic>

namespace toolbox {
inline namespace util {

struct ThreadSafePolicy {
    using Type = std::atomic<int>;
    static void acquire() noexcept { std::atomic_thread_fence(std::memory_order_acquire); }
    static int fetch_add(Type& ref_count) noexcept
    {
        return ref_count.fetch_add(1, std::memory_order_relaxed);
    }
    static int fetch_sub(Type& ref_count) noexcept
    {
        return ref_count.fetch_sub(1, std::memory_order_release);
    }
    static int load(const Type& ref_count) noexcept
    {
        return ref_count.load(std::memory_order_relaxed);
    }
};

struct ThreadUnsafePolicy {
    using Type = int;
    static void acquire() noexcept {}
    static int fetch_add(Type& ref_count) noexcept { return ref_count++; }
    static int fetch_sub(Type& ref_count) noexcept { return ref_count--; }
    static int load(Type ref_count) noexcept { return ref_count; }
};

/// Base class for atomic referenced counted objects.
template <typename DerivedT, typename PolicyT>
class RefCount {
  public:
    constexpr RefCount() noexcept = default;
    ~RefCount() = default;

    // Copy.
    constexpr RefCount(const RefCount&) noexcept = default;
    RefCount& operator=(const RefCount&) noexcept = default;

    // Move.
    constexpr RefCount(RefCount&&) noexcept = default;
    RefCount& operator=(RefCount&&) noexcept = default;

    void add_ref() const noexcept { PolicyT::fetch_add(refs_); }
    void release() const noexcept
    {
        if (PolicyT::fetch_sub(refs_) == 1) {
            PolicyT::acquire();
            delete static_cast<const DerivedT*>(this);
        }
    }
    int ref_count() const noexcept { return PolicyT::load(refs_); }

  private:
    mutable typename PolicyT::Type refs_{1};
};

template <typename DerivedT, typename PolicyT>
void intrusive_ptr_add_ref(const RefCount<DerivedT, PolicyT>* ptr) noexcept
{
    ptr->add_ref();
}

template <typename DerivedT, typename PolicyT>
void intrusive_ptr_release(const RefCount<DerivedT, PolicyT>* ptr) noexcept
{
    ptr->release();
}

template <typename ValueT, typename... ArgsT>
boost::intrusive_ptr<ValueT> make_intrusive(ArgsT&&... args)
{
    return {new ValueT{std::forward<ArgsT>(args)...}, false};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_REFCOUNT_HPP
