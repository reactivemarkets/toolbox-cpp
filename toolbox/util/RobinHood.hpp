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

#ifndef TOOLBOX_UTIL_ROBINHOOD_HPP
#define TOOLBOX_UTIL_ROBINHOOD_HPP

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <toolbox/contrib/robin_hood.h>
#pragma GCC diagnostic pop

namespace toolbox {
inline namespace util {

/// Robin Hood hash function template.
template <typename KeyT>
using RobinHash = robin_hood::hash<KeyT>;

/// Robin Hood unordered map.
///
/// Robin Hood unordered map is a replacement for std::unordered_map which is both faster and more
/// memory efficient for real-world use cases.
template <typename KeyT, typename ValueT, typename HashT = RobinHash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>, std::size_t MaxLoadFactor100N = 80>
using RobinMap = robin_hood::unordered_map<KeyT, ValueT, HashT, KeyEqualT, MaxLoadFactor100N>;

/// Robin Hood unordered flat map.
template <typename KeyT, typename ValueT, typename HashT = RobinHash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>, std::size_t MaxLoadFactor100N = 80>
using RobinFlatMap
    = robin_hood::unordered_flat_map<KeyT, ValueT, HashT, KeyEqualT, MaxLoadFactor100N>;

/// Robin Hood unordered node map.
template <typename KeyT, typename ValueT, typename HashT = RobinHash<KeyT>,
          typename KeyEqualT = std::equal_to<KeyT>, std::size_t MaxLoadFactor100N = 80>
using RobinNodeMap = robin_hood::unordered_map<KeyT, ValueT, HashT, KeyEqualT, MaxLoadFactor100N>;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ROBINHOOD_HPP
