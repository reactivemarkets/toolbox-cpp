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

#ifndef TOOLBOX_UTIL_ARRAY_HPP
#define TOOLBOX_UTIL_ARRAY_HPP

#include <cstddef> // ptrdiff_t
#include <iterator>
#include <vector>

namespace toolbox {
inline namespace util {

template <typename ValueT, std::size_t SizeN>
constexpr std::size_t array_size(const ValueT (&)[SizeN]) noexcept
{
    return SizeN;
}

template <typename ValueT>
class ArrayView {
  public:
    using value_type = ValueT;

    using pointer = const ValueT*;
    using const_pointer = const ValueT*;

    using reference = const ValueT&;
    using const_reference = const ValueT&;

    using iterator = const ValueT*;
    using const_iterator = const ValueT*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    constexpr ArrayView(const ValueT* ptr, std::size_t len) noexcept
    : len_{len}
    , ptr_{ptr}
    {
    }
    template <typename TypeU, std::size_t SizeN>
    constexpr ArrayView(TypeU (&arr)[SizeN]) noexcept
    : len_{SizeN}
    , ptr_{arr}
    {
    }
    ArrayView(const std::vector<ValueT>& arr) noexcept
    : len_{arr.size()}
    , ptr_{arr.empty() ? nullptr : &arr[0]}
    {
    }
    constexpr ArrayView() noexcept = default;
    ~ArrayView() = default;

    // Copy.
    constexpr ArrayView(const ArrayView&) noexcept = default;
    constexpr ArrayView& operator=(const ArrayView&) noexcept = default;

    // Move.
    constexpr ArrayView(ArrayView&&) noexcept = default;
    constexpr ArrayView& operator=(ArrayView&&) noexcept = default;

    constexpr const_iterator begin() const noexcept { return ptr_; }
    constexpr const_iterator end() const noexcept { return ptr_ + len_; }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr const_iterator cbegin() const noexcept { return ptr_; }
    constexpr const_iterator cend() const noexcept { return ptr_ + len_; }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr const_reference operator[](size_type pos) const noexcept { return ptr_[pos]; }
    constexpr const_reference front() const noexcept { return ptr_[0]; }
    constexpr const_reference back() const noexcept { return ptr_[len_ - 1]; }
    constexpr const ValueT* data() const noexcept { return ptr_; }
    constexpr bool empty() const noexcept { return len_ == 0; }
    constexpr std::size_t size() const noexcept { return len_; }
    constexpr void clear() noexcept
    {
        len_ = 0;
        ptr_ = nullptr;
    }
    void swap(ArrayView& rhs) noexcept
    {
        std::swap(len_, rhs.len_);
        std::swap(ptr_, rhs.ptr_);
    }

  private:
    // Length in the first cache-line.
    std::size_t len_{0};
    const ValueT* ptr_{nullptr};
};

template <typename ValueT>
constexpr ArrayView<std::remove_volatile_t<ValueT>> make_array_view(const ValueT* ptr,
                                                                    std::size_t len) noexcept
{
    return {ptr, len};
}

template <typename ValueT, std::size_t SizeN>
constexpr ArrayView<std::remove_cv_t<ValueT>> make_array_view(ValueT (&arr)[SizeN]) noexcept
{
    return {arr};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ARRAY_HPP
