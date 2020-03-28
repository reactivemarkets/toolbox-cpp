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

#include "VarSub.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

template <typename FnT>
string apply_copy(FnT fn, string s)
{
    fn(s);
    return s;
}

string get_var(const string& name)
{
    string val;
    if (name == "FOO") {
        val = "101";
    } else if (name == "BAR") {
        val = "202";
    } else if (name == "BAZ") {
        val = "FOO";
    } else if (name == "QUX") {
        val = "BAR";
    } else if (name == "FOOBAR") {
        val = "${FOO}${BAR}";
    }
    return val;
}

} // namespace

BOOST_AUTO_TEST_SUITE(VarSubSuite)

BOOST_AUTO_TEST_CASE(VarSubBasicCase)
{
    VarSub fn{get_var};

    BOOST_TEST(apply_copy(fn, "${FOO}") == "101");
    BOOST_TEST(apply_copy(fn, "${BAR}") == "202");
    BOOST_TEST(apply_copy(fn, "<${FOO}>") == "<101>");
    BOOST_TEST(apply_copy(fn, "<${FOO} ${BAR}>") == "<101 202>");
}

BOOST_AUTO_TEST_CASE(VarSubEmptyCase)
{
    VarSub fn{get_var};

    BOOST_TEST(apply_copy(fn, "${}").empty());
    BOOST_TEST(apply_copy(fn, "${123}").empty());
    BOOST_TEST(apply_copy(fn, "${EMPTY}").empty());
}

BOOST_AUTO_TEST_CASE(VarSubEscapeCase)
{
    VarSub fn{get_var};

    BOOST_TEST(apply_copy(fn, "\\\\") == "\\");
    BOOST_TEST(apply_copy(fn, "\\\\>") == "\\>");
    BOOST_TEST(apply_copy(fn, "<\\\\") == "<\\");
    BOOST_TEST(apply_copy(fn, "\\${FOO}") == "${FOO}");
    BOOST_TEST(apply_copy(fn, "$\\{FOO}") == "${FOO}");
    BOOST_TEST(apply_copy(fn, "${\\FOO}") == "101");
    BOOST_TEST(apply_copy(fn, "${FOO\\}") == "${FOO}");
    BOOST_TEST(apply_copy(fn, "${FOO}\\") == "101\\");
}

BOOST_AUTO_TEST_CASE(VarSubPartialCase)
{
    VarSub fn{get_var};

    BOOST_TEST(apply_copy(fn, "$") == "$");
    BOOST_TEST(apply_copy(fn, "{") == "{");
    BOOST_TEST(apply_copy(fn, "}") == "}");
    BOOST_TEST(apply_copy(fn, "$FOO") == "$FOO");
    BOOST_TEST(apply_copy(fn, "{FOO") == "{FOO");
    BOOST_TEST(apply_copy(fn, "${FOO") == "${FOO");
    BOOST_TEST(apply_copy(fn, "FOO}") == "FOO}");
    BOOST_TEST(apply_copy(fn, "$${FOO}") == "$101");
}

BOOST_AUTO_TEST_CASE(VarSubNestedCase)
{
    VarSub fn{get_var};

    BOOST_TEST(apply_copy(fn, "${FOOBAR}") == "101202");
    BOOST_TEST(apply_copy(fn, "${${BAZ}}") == "101");
    BOOST_TEST(apply_copy(fn, "${${BAZ}BAR}") == "101202");
    BOOST_TEST(apply_copy(fn, "${FOO${QUX}}") == "101202");
    BOOST_TEST(apply_copy(fn, "${${BAZ}${QUX}}") == "101202");
}

BOOST_AUTO_TEST_CASE(VarSubLoopCase)
{
    VarSub fn{[](const string& name) {
        string val;
        if (name == "FOO") {
            val = "${BAR}";
        } else if (name == "BAR") {
            val = "${BAZ}";
        } else if (name == "BAZ") {
            val = "${QUX}";
        } else if (name == "QUX") {
            val = "${FOO}";
        }
        return val;
    }};

    BOOST_TEST(apply_copy(fn, "${FOO}").empty());
    BOOST_TEST(apply_copy(fn, "${FOO}${FOO}").empty());
    BOOST_TEST(apply_copy(fn, "${FOO${FOO}}").empty());
    BOOST_TEST(apply_copy(fn, "<${FOO}>") == "<>");
    BOOST_TEST(apply_copy(fn, "<${FOO${FOO}}>") == "<>");
    BOOST_TEST(apply_copy(fn, "<${FOO} ${FOO}>") == "< >");
}

BOOST_AUTO_TEST_SUITE_END()
