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

#ifndef TOOLBOX_UTIL_STORAGE_HPP
#define TOOLBOX_UTIL_STORAGE_HPP

#include <toolbox/util/Allocator.hpp>

#include <iterator>

namespace toolbox {
inline namespace util {

/// Storage represents a dynamic block of storage acquired from the custom allocator.
template <std::size_t SizeN>
class Storage : public Allocator {
  public:
    using value_type = char;

    using pointer = char*;
    using const_pointer = const char*;

    using reference = char&;
    using const_reference = const char&;

    using iterator = char*;
    using const_iterator = const char*;

    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    using difference_type = std::ptrdiff_t;
    using size_type = std::size_t;

    constexpr iterator begin() noexcept { return iterator(data()); }
    constexpr const_iterator begin() const noexcept { return const_iterator(data()); }
    constexpr iterator end() noexcept { return iterator(data() + SizeN); }
    constexpr const_iterator end() const noexcept { return const_iterator(data() + SizeN); }
    constexpr reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
    constexpr const_reverse_iterator rbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
    constexpr const_reverse_iterator rend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr const_iterator cbegin() const noexcept { return const_iterator(data()); }
    constexpr const_iterator cend() const noexcept { return const_iterator(data() + SizeN); }
    constexpr const_reverse_iterator crbegin() const noexcept
    {
        return const_reverse_iterator(end());
    }
    constexpr const_reverse_iterator crend() const noexcept
    {
        return const_reverse_iterator(begin());
    }
    constexpr size_type size() const noexcept { return SizeN; }
    constexpr size_type max_size() const noexcept { return SizeN; }
    constexpr reference operator[](size_type pos) noexcept { return data_[pos]; }
    constexpr const_reference operator[](size_type pos) const noexcept { return data_[pos]; }
    constexpr pointer data() noexcept { return data_; }
    constexpr const_pointer data() const noexcept { return data_; }

  private:
    char data_[SizeN];
};

template <std::size_t SizeN>
using StoragePtr = std::unique_ptr<Storage<SizeN>>;

/// Returns a block of dynamic block of storage acquired from the custom allocator.
template <std::size_t SizeN>
StoragePtr<SizeN> make_storage()
{
    return std::unique_ptr<Storage<SizeN>>{new Storage<SizeN>};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STORAGE_HPP
