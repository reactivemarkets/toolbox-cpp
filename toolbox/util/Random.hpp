// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2024 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_RANDOM_HPP
#define TOOLBOX_UTIL_RANDOM_HPP

#include <toolbox/Config.h>

#include <random>

namespace toolbox {
inline namespace util {

TOOLBOX_API std::mt19937_64& mt19937_64_rng() noexcept;

template <class IntT>
IntT randint(IntT a, IntT b) {
    std::mt19937_64& rng_engine = mt19937_64_rng();
    std::uniform_int_distribution<IntT> dist(a, b);
    return dist(rng_engine);
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_RANDOM_HPP
