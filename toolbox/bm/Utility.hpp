// The Reactive C++ Toolbox.
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

#ifndef TOOLBOX_BM_UTILITY_HPP
#define TOOLBOX_BM_UTILITY_HPP

#include <toolbox/Config.h>

namespace toolbox::bm {

// The clobber_memory and do_not_optimise functions below are used to defeat the optimiser.
// See CppCon 2015: Chandler Carruth "Tuning C++: Benchmarks, and CPUs, and Compilers! Oh My!"
// And https://github.com/google/benchmark/blob/master/include/benchmark/benchmark.h

/// The clobber_memory function instructs the compiler that something might have changed in the
/// memory and no assumptions can be made that would discard memory writes.
inline void clobber_memory() noexcept
{
    asm volatile("" ::: "memory");
}

/// The do_not_optimise function emulates the creation additional uses of the reference essentially
/// instructing the compiler that pointers in it's scope are now available to call though global
/// pointers.
template <typename ValueT>
inline void do_not_optimise(const ValueT& val) noexcept
{
    asm volatile("" : : "r,m"(val) : "memory");
}

/// The do_not_optimise function emulates the creation additional uses of the reference essentially
/// instructing the compiler that pointers in it's scope are now available to call though global
/// pointers.
template <typename ValueT>
inline void do_not_optimise(ValueT& val) noexcept
{
#if defined(__clang__)
    asm volatile("" : "+r,m"(val) : : "memory");
#else
    asm volatile("" : "+m,r"(val) : : "memory");
#endif
}
} // namespace toolbox::bm

#endif // TOOLBOX_BM_UTILITY_HPP
