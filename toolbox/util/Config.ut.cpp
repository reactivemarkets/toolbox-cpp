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

#include "Config.hpp"

#include <boost/test/unit_test.hpp>

#include <map>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(ConfigSuite)

BOOST_AUTO_TEST_CASE(ParseSectionSingleCase)
{
    const string text{R"(
# comment
  # indented comment
ab
cd=
ef=gh
=ij

kl = mn
 op = qr 
st = = uv =

)"};

    istringstream is{text};
    map<string, string> conf;
    parse_section(is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); });
    BOOST_TEST(conf.size() == 7U);
    BOOST_TEST(conf["ab"] == "");
    BOOST_TEST(conf["cd"] == "");
    BOOST_TEST(conf["ef"] == "gh");
    BOOST_TEST(conf[""] == "ij");
    BOOST_TEST(conf["kl"] == "mn");
    BOOST_TEST(conf["op"] == "qr");
    BOOST_TEST(conf["st"] == "= uv =");
    BOOST_TEST(is.eof());
}

BOOST_AUTO_TEST_CASE(ParseSectionMultiCase)
{
    const string text{R"(
# comment
  # indented comment
ab
cd=

[foo]
ef=gh
=ij

 [ bar ] 
kl = mn
 op = qr 
st = = uv =

)"};

    istringstream is{text};
    map<string, string> conf;

    string next;
    parse_section(
        is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); }, &next);
    BOOST_TEST(conf.size() == 2U);
    BOOST_TEST(conf["ab"] == "");
    BOOST_TEST(conf["cd"] == "");
    BOOST_TEST(next == "foo");
    BOOST_TEST(!is.fail());

    conf.clear();
    parse_section(
        is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); }, &next);
    BOOST_TEST(conf.size() == 2U);
    BOOST_TEST(conf["ef"] == "gh");
    BOOST_TEST(conf[""] == "ij");
    BOOST_TEST(next == "bar");
    BOOST_TEST(!is.fail());

    conf.clear();
    parse_section(
        is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); }, &next);
    BOOST_TEST(conf.size() == 3U);
    BOOST_TEST(conf["kl"] == "mn");
    BOOST_TEST(conf["op"] == "qr");
    BOOST_TEST(conf["st"] == "= uv =");
    BOOST_TEST(next.empty());
    BOOST_TEST(is.eof());
}

BOOST_AUTO_TEST_CASE(ConfigOverrideCase)
{
    const string text{R"(
foo=101
bar=202

[session]

bar=303
baz=404
)"};

    istringstream is{text};
    string next;

    Config parent;
    parent.read_section(is, next);

    BOOST_TEST(parent.size() == 2U);
    BOOST_TEST(parent.get<int>("foo", 0) == 101);
    BOOST_TEST(parent.get<int>("bar", 0) == 202);
    BOOST_TEST(next == "session");
    BOOST_TEST(!is.fail());

    // Verify that getter with nullptr default compiles.
    BOOST_TEST(parent.get("foo", nullptr));

    // Conversion from internal std::string to std::string_view is a special case.
    BOOST_TEST(parent.get<string_view>("foo") == "101"sv);

    Config child;
    child.read_section(is, next);
    child.set_parent(parent);

    BOOST_TEST(child.size() == 2U);
    BOOST_TEST(child.get<int>("foo", 0) == 101);
    BOOST_TEST(child.get<int>("bar", 0) == 303);
    BOOST_TEST(child.get<int>("baz", 0) == 404);
    BOOST_TEST(next.empty());
    BOOST_TEST(is.eof());

    BOOST_CHECK_THROW(child.get("bad"), runtime_error);
    BOOST_CHECK_THROW(child.get<int>("bad"), runtime_error);
}

BOOST_AUTO_TEST_SUITE_END()
