// The Reactive C++ Toolbox.
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

#include "StreamInserter.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {
auto start_tag(const char* s)
{
    return StreamInserter{[=](ostream& os) { os << '<' << s << '>'; }};
}
auto end_tag(const char* s)
{
    return StreamInserter{[=](ostream& os) { os << "</" << s << '>'; }};
}
} // namespace

BOOST_AUTO_TEST_SUITE(StreamInserterSuite)

BOOST_AUTO_TEST_CASE(StreamInserterCase)
{
    stringstream ss;
    ss << start_tag("foo") << "bar" << end_tag("foo");
    BOOST_TEST(ss.str() == "<foo>bar</foo>"s);
}

BOOST_AUTO_TEST_SUITE_END()
