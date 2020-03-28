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

#include "RefCount.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
class Foo : public RefCount<Foo, ThreadUnsafePolicy> {
  public:
    explicit Foo(int& alive) noexcept
    : alive_{alive}
    {
        ++alive;
    }
    ~Foo() { --alive_; }

  private:
    int& alive_;
};
} // namespace

BOOST_AUTO_TEST_SUITE(RefCountSuite)

BOOST_AUTO_TEST_CASE(RefCountCase)
{
    int alive{0};
    {
        auto ptr1 = make_intrusive<Foo>(alive);
        BOOST_TEST(alive == 1);
        BOOST_TEST(ptr1->ref_count() == 1);
        {
            auto ptr2 = ptr1;
            BOOST_TEST(ptr1->ref_count() == 2);
        }
        BOOST_TEST(ptr1->ref_count() == 1);
    }
    BOOST_TEST(alive == 0);
}

BOOST_AUTO_TEST_SUITE_END()
