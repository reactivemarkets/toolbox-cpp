// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#include "Debug.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(DebugSuite)

BOOST_AUTO_TEST_CASE(DumpCase)
{
    std::string data{"12345"};
    {
        stringstream ss;
        ss << hex_dump(data.c_str(), data.size() + 1);

        BOOST_TEST(ss.str() == " 0x31 0x32 0x33 0x34 0x35 0x00");
    }
    {
        stringstream ss;
        ss << hex_dump(data.c_str(), data.size() + 1, hex_dump::Mode::NON_PRINTABLE);

        BOOST_TEST(ss.str() == " 1 2 3 4 5 0x00");
    }
}

BOOST_AUTO_TEST_SUITE_END()
