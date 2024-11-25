// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#include "Config.hpp"

#include <boost/test/unit_test.hpp>

#include <map>
#include <stdexcept>

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
    BOOST_CHECK_EQUAL(conf.size(), 7U);
    BOOST_CHECK_EQUAL(conf["ab"], "");
    BOOST_CHECK_EQUAL(conf["cd"], "");
    BOOST_CHECK_EQUAL(conf["ef"], "gh");
    BOOST_CHECK_EQUAL(conf[""], "ij");
    BOOST_CHECK_EQUAL(conf["kl"], "mn");
    BOOST_CHECK_EQUAL(conf["op"], "qr");
    BOOST_CHECK_EQUAL(conf["st"], "= uv =");
    BOOST_CHECK(is.eof());
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
    BOOST_CHECK_EQUAL(conf.size(), 2U);
    BOOST_CHECK_EQUAL(conf["ab"], "");
    BOOST_CHECK_EQUAL(conf["cd"], "");
    BOOST_CHECK_EQUAL(next, "foo");
    BOOST_CHECK(!is.fail());

    conf.clear();
    parse_section(
        is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); }, &next);
    BOOST_CHECK_EQUAL(conf.size(), 2U);
    BOOST_CHECK_EQUAL(conf["ef"], "gh");
    BOOST_CHECK_EQUAL(conf[""], "ij");
    BOOST_CHECK_EQUAL(next, "bar");
    BOOST_CHECK(!is.fail());

    conf.clear();
    parse_section(
        is, [&conf](const auto& key, const auto& val) { conf.emplace(key, val); }, &next);
    BOOST_CHECK_EQUAL(conf.size(), 3U);
    BOOST_CHECK_EQUAL(conf["kl"], "mn");
    BOOST_CHECK_EQUAL(conf["op"], "qr");
    BOOST_CHECK_EQUAL(conf["st"], "= uv =");
    BOOST_CHECK(next.empty());
    BOOST_CHECK(is.eof());
}

BOOST_AUTO_TEST_CASE(ConfigSetAndGetCase)
{
    Config config;

    BOOST_CHECK_EQUAL(config.get<int>("foo", 101), 101);
    BOOST_CHECK_EQUAL(config.get<int>("bar", 202), 202);

    config.set("foo", "101");
    config.set("bar", "202");

    BOOST_CHECK_EQUAL(config.get<int>("foo", 0), 101);
    BOOST_CHECK_EQUAL(config.get<int>("bar", 0), 202);

    config.set("foo", "303");

    BOOST_CHECK_EQUAL(config.get<int>("foo", 0), 303);
    BOOST_CHECK_EQUAL(config.get<int>("bar", 0), 202);
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

    BOOST_CHECK_EQUAL(parent.size(), 2U);
    BOOST_CHECK_EQUAL(parent.get<int>("foo", 0), 101);
    BOOST_CHECK_EQUAL(parent.get<int>("bar", 0), 202);
    BOOST_CHECK_EQUAL(next, "session");
    BOOST_CHECK(!is.fail());

    // Verify that getter with nullptr default compiles.
    BOOST_CHECK(parent.get("foo", nullptr));

    // Conversion from internal std::string to std::string_view is a special case.
    BOOST_CHECK_EQUAL(parent.get<string_view>("foo"), "101"sv);

    Config child;
    child.read_section(is, next);
    child.set_parent(parent);

    BOOST_CHECK_EQUAL(child.size(), 2U);
    BOOST_CHECK_EQUAL(child.get<int>("foo", 0), 101);
    BOOST_CHECK_EQUAL(child.get<int>("bar", 0), 303);
    BOOST_CHECK_EQUAL(child.get<int>("baz", 0), 404);
    BOOST_CHECK(next.empty());
    BOOST_CHECK(is.eof());

    BOOST_CHECK_THROW(child.get("bad"), runtime_error);
    BOOST_CHECK_THROW(child.get<int>("bad"), runtime_error);
}

BOOST_AUTO_TEST_CASE(ConfigEscapeCase)
{
    const string text{R"(
foo=\a\b\c
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 1U);
    BOOST_CHECK_EQUAL(config.get("foo"), "abc");
}

BOOST_AUTO_TEST_CASE(SyntaxEdgeCases)
{
    const string text{R"(
foo = test=++=test
foo +  = test=++=test
baz1
baz2=
baz3  +=
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 5U);
    BOOST_CHECK_EQUAL(config.get("foo"), "test=++=test");
    BOOST_CHECK_EQUAL(config.get("foo +"), "test=++=test");
    BOOST_CHECK_EQUAL(config.get("baz1"), "");
    BOOST_CHECK_EQUAL(config.get("baz2"), "");
    BOOST_CHECK_EQUAL(config.get("baz3"), "");
}

BOOST_AUTO_TEST_CASE(SyntaxEdgeCases2)
{
    const string text{R"(
foo += test=++=test
foo +==abcd
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 2U);

    auto rng = config.get_multi("foo");
    BOOST_CHECK_EQUAL(rng.size(), 2);
    
    auto it =  rng.begin();
    BOOST_CHECK_EQUAL(*it++, "test=++=test");
    BOOST_CHECK_EQUAL(*it++, "=abcd");
    BOOST_CHECK_EQUAL(it == rng.end(), true);
}

BOOST_AUTO_TEST_CASE(MultipleValuesForKeyTextualIter)
{
    const string text{R"(
foo+=101
foo+=202
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 2U);
    BOOST_CHECK_THROW(config.get<int>("foo", 0), runtime_error);

    auto rng = config.get_multi("foo");
    BOOST_CHECK_EQUAL(rng.size(), 2);

    auto it = rng.begin();
    BOOST_CHECK_EQUAL(*it++, "101");
    BOOST_CHECK_EQUAL(*it++, "202");
    BOOST_CHECK_EQUAL(it == rng.end(), true);
}

BOOST_AUTO_TEST_CASE(MultipleValuesForKeyTypedIntIter)
{
    const string text{R"(
foo+=101
foo+=202
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 2U);
    BOOST_CHECK_THROW(config.get<int>("foo", 0), runtime_error);

    auto rng = config.get_multi<int>("foo");
    BOOST_CHECK_EQUAL(rng.size(), 2);

    auto it = rng.begin();
    BOOST_CHECK_EQUAL(*it++, 101);
    BOOST_CHECK_EQUAL(*it++, 202);
    BOOST_CHECK_EQUAL(it == rng.end(), true);
}

BOOST_AUTO_TEST_CASE(MultipleValuesForKeyTypedStringViewIter)
{
    const string text{R"(
foo+=101
foo+=202
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 2U);
    BOOST_CHECK_THROW(config.get<int>("foo", 0), runtime_error);

    auto rng = config.get_multi<string_view>("foo");
    BOOST_CHECK_EQUAL(rng.size(), 2);

    auto it = rng.begin();
    BOOST_CHECK_EQUAL(*it++, "101"sv);
    BOOST_CHECK_EQUAL(*it++, "202"sv);
    BOOST_CHECK_EQUAL(it == rng.end(), true);
}

BOOST_AUTO_TEST_CASE(MultipleValuesWithSingleValueSyntax)
{
    const string text{R"(
foo=101
foo=202
)"};

    istringstream is{text};

    Config config;
    BOOST_CHECK_THROW(config.read_section(is), runtime_error);
}

BOOST_AUTO_TEST_CASE(ReassignSingleValuedKey)
{
    const string text{R"(
foo=101
foo+=202
)"};

    istringstream is{text};

    Config config;
    BOOST_CHECK_THROW(config.read_section(is), runtime_error);
}

BOOST_AUTO_TEST_CASE(ReassignMultiValuedKey)
{
    const string text{R"(
foo+=101
foo=202
)"};

    istringstream is{text};

    Config config;
    BOOST_CHECK_THROW(config.read_section(is), runtime_error);
}

BOOST_AUTO_TEST_CASE(MultiAPIWithZeroValueKey)
{
    const string text{R"()"};
    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 0U);

    auto rng = config.get_multi<string_view>("foo");
    BOOST_CHECK_EQUAL(rng.empty(), true);
    BOOST_CHECK_EQUAL(rng.size(), 0U);
    BOOST_CHECK_EQUAL(rng.begin() == rng.end(), true);
}


BOOST_AUTO_TEST_CASE(MultiAPIWithOneValueKey)
{
    const string text{R"(
foo=101
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 1U);

    auto rng = config.get_multi<string_view>("foo");
    BOOST_CHECK_EQUAL(rng.size(), 1);

    BOOST_CHECK_EQUAL(*rng.begin(), "101"sv);
}

BOOST_AUTO_TEST_CASE(InsertAPI)
{
    const string text{R"(
foo=101
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 1U);
    BOOST_CHECK_EQUAL(config.get<int>("foo"), 101);

    config.insert("bar", "202");
    config.insert("foo", "102");

    BOOST_CHECK_EQUAL(config.get("bar"), "202");
    BOOST_CHECK_EQUAL(config.get<int>("bar"), 202);
    BOOST_CHECK_THROW(config.get<int>("foo"), runtime_error);

    auto foo_rng = config.get_multi<int>("foo");
    BOOST_CHECK_EQUAL(foo_rng.size(), 2);
    
    auto foo_it = foo_rng.begin();
    BOOST_CHECK_EQUAL(*foo_it++, 101);
    BOOST_CHECK_EQUAL(*foo_it++, 102);
    BOOST_CHECK_EQUAL(foo_it == foo_rng.end(), true);
}

BOOST_AUTO_TEST_CASE(SetAPI)
{
    const string text{R"(
foo=101
)"};

    istringstream is{text};

    Config config;
    config.read_section(is);

    BOOST_CHECK_EQUAL(config.size(), 1U);
    BOOST_CHECK_EQUAL(config.get<int>("foo"), 101);

    config.set("bar", "100");
    config.set("foo", "111", "222", "333", "444");

    BOOST_CHECK_EQUAL(config.get("bar"), "100");
    BOOST_CHECK_EQUAL(config.get<int>("bar"), 100);
    BOOST_CHECK_THROW(config.get<int>("foo"), runtime_error);

    auto foo_rng = config.get_multi<int>("foo");
    BOOST_CHECK_EQUAL(foo_rng.size(), 4);
    
    auto foo_it = foo_rng.begin();
    BOOST_CHECK_EQUAL(*foo_it++, 111);
    BOOST_CHECK_EQUAL(*foo_it++, 222);
    BOOST_CHECK_EQUAL(*foo_it++, 333);
    BOOST_CHECK_EQUAL(*foo_it++, 444);
    BOOST_CHECK_EQUAL(foo_it == foo_rng.end(), true);
}

BOOST_AUTO_TEST_SUITE_END()
