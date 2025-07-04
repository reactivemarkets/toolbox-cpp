// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#include <memory>

namespace toolbox {
inline namespace util {
namespace detail {
template <std::size_t SizeN>
struct StorageDeleter {
    void operator()(char* ptr) const noexcept { delete[] ptr; }
};
} // namespace detail

/// Pointer to an uninitialised char array of SizeN, freed with delete[].
template <std::size_t SizeN>
using StoragePtr = std::unique_ptr<char[], detail::StorageDeleter<SizeN>>;

/// Returns a block of dynamic storage.
template <std::size_t SizeN>
StoragePtr<SizeN> make_storage()
{
    return StoragePtr<SizeN>{new char[SizeN]};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STORAGE_HPP
