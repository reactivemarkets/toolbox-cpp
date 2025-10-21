// The Reactive C++ Toolbox.
// Copyright (C) 2025 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_PROPAGATECONST_HPP
#define TOOLBOX_UTIL_PROPAGATECONST_HPP

#include <memory>
#include <utility>

#include <toolbox/util/Concepts.hpp>

namespace toolbox {
inline namespace util {

// user may specialise this for custom types
template <typename T>
struct pointed_underlying;

template <typename T>
    requires requires(T x) { { x.get() } -> Pointer; }
struct pointed_underlying<T> {
    auto* operator()(T& obj) { return obj.get(); }
};

template <typename T>
struct pointed_underlying<T*> {
    T* operator()(T* obj) { return obj; }
};

template <typename T>
struct pointed_underlying<T&> {
    T* operator()(T& obj) { return std::addressof(obj); }
};

template <class T>
class propagate_const {
    using pointed_type = std::remove_pointer_t<
        decltype(pointed_underlying<T>{}(std::declval<T&>()))
    >;

  public:
    propagate_const() = default;
    ~propagate_const() = default;

    explicit propagate_const(T obj)
    : obj_(std::move(obj))
    {
    }

    propagate_const(propagate_const& o) = default;
    propagate_const& operator=(propagate_const&) = default;

    propagate_const(const propagate_const&) = delete;
    propagate_const& operator=(const propagate_const&) = delete;

    propagate_const(propagate_const&&) = default;
    propagate_const& operator=(propagate_const&&) = default;

    propagate_const& operator=(T&& other)
    {
        obj_ = std::move(other);
        return *this;
    }
    propagate_const& operator=(const T& other)
    {
        obj_ = other;
        return *this;
    }

    pointed_type* get() { return pointed_underlying<T>{}(obj_); }
    const pointed_type* get() const { return pointed_underlying<T>{}(obj_); }

    pointed_type* operator->() { return get(); }
    const pointed_type* operator->() const { return get(); }

    pointed_type& operator*() { return *get(); }
    const pointed_type& operator*() const { return *get(); }

    operator pointed_type*() { return get(); }
    operator const pointed_type*() const { return get(); }

    explicit operator bool() const { return get() != nullptr; }

    friend bool operator==(const propagate_const&, const propagate_const&) = default;
    friend auto operator<=>(const propagate_const&, const propagate_const&) = default;

    template <class U>
    friend auto operator<=>(const propagate_const& p, const U& o)
    {
        return p.obj_ <=> o;
    }
    template <typename U>
    friend auto operator==(const propagate_const& p, const U& o)
    {
        return p.obj_ == o;
    }

    friend bool operator==(const propagate_const& p, std::nullptr_t) { return p.get() == nullptr; }

    friend void swap(propagate_const& a, propagate_const& b)
    {
        using std::swap;
        swap(a.obj_, b.obj_);
    }

  private:
    T obj_;
};

} // namespace util
} // namespace toolbox

#endif
