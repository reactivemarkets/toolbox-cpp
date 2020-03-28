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

#include "Version.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
} // namespace

BOOST_AUTO_TEST_SUITE(VersionSuite)

BOOST_AUTO_TEST_CASE(VersionFromStringCase)
{
    BOOST_TEST(from_string<Version>("0.0"sv) == Version());
    BOOST_TEST(from_string<Version>("1.0"sv) == Version(1));
    BOOST_TEST(from_string<Version>("1.2"sv) == Version(1, 2));
}

BOOST_AUTO_TEST_SUITE_END()
