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

#include "MemAlloc.hpp"

#include <toolbox/util/RefCount.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

std::size_t size_{};
std::align_val_t al_{};

struct alignas(64) Foo : RefCount<Foo, ThreadUnsafePolicy> {

    static void* operator new(std::size_t size, std::align_val_t al)
    {
        size_ = size;
        al_ = al;
        return ::operator new(size, al);
    }
    static void operator delete(void* ptr, std::size_t size, std::align_val_t al) noexcept
    {
        size_ = size;
        al_ = al;
#if __cpp_sized_deallocation
        ::operator delete(ptr, size, al);
#else
        ::operator delete(ptr, al);
#endif
    }
    char data[64 + 1];
};

} // namespace

BOOST_AUTO_TEST_SUITE(MemAllocSuite)

BOOST_AUTO_TEST_CASE(MemAllocCase)
{
    {
        auto h = make_intrusive<Foo>();
        BOOST_TEST(size_ == 128);
        BOOST_TEST(static_cast<int>(al_) == 64);
        size_ = {};
        al_ = {};
    }
    BOOST_TEST(size_ == 128);
    BOOST_TEST(static_cast<int>(al_) == 64);
}

BOOST_AUTO_TEST_SUITE_END()
