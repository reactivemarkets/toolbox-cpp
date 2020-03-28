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

#include "Tokeniser.hpp"

#include <boost/test/unit_test.hpp>

using namespace toolbox;

BOOST_AUTO_TEST_SUITE(TokeniserSuite)

BOOST_AUTO_TEST_CASE(TokeniserEmptyCase)
{
    Tokeniser toks;

    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());

    toks = Tokeniser{""sv, ","sv};
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
}

BOOST_AUTO_TEST_CASE(TokeniserMultiEmptyCase)
{
    Tokeniser toks{","sv, ","sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top().empty());

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserSingleValueCase)
{
    Tokeniser toks{"foo"sv, ","sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top() == "foo");

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserMultiValueCase)
{
    Tokeniser toks{"foo,bar,baz"sv, ","sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top() == "foo");

    toks.pop();
    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top() == "bar");

    toks.pop();
    BOOST_TEST(!toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top() == "baz");

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserMultiSepCase)
{
    Tokeniser toks{"foo,bar;baz"sv, ",;"sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top() == "foo");

    toks.pop();
    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top() == "bar");

    toks.pop();
    BOOST_TEST(!toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top() == "baz");

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserLeadingCase)
{
    Tokeniser toks{",foo"sv, ","sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top().empty());

    toks.pop();
    BOOST_TEST(!toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top() == "foo");

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserTrailingCase)
{
    Tokeniser toks{"foo,"sv, ","sv};

    BOOST_TEST(!toks.empty());
    BOOST_TEST(toks.has_delim());
    BOOST_TEST(toks.top() == "foo");

    toks.pop();
    BOOST_TEST(toks.empty());
    BOOST_TEST(!toks.has_delim());
    BOOST_TEST(toks.top().empty());
}

BOOST_AUTO_TEST_CASE(TokeniserLineCase)
{
    Tokeniser lines{"foo\nbar\nbaz..."sv, "\n"sv};

    BOOST_TEST(!lines.empty());
    BOOST_TEST(lines.has_delim());
    BOOST_TEST(lines.top() == "foo");

    lines.pop();
    BOOST_TEST(!lines.empty());
    BOOST_TEST(lines.has_delim());
    BOOST_TEST(lines.top() == "bar");

    lines.pop();
    BOOST_TEST(!lines.empty());
    BOOST_TEST(!lines.has_delim());
    BOOST_TEST(lines.top() == "baz...");
    BOOST_TEST(lines.consumed() == 8);

    lines.pop();
    BOOST_TEST(lines.empty());
    BOOST_TEST(!lines.has_delim());
    BOOST_TEST(lines.top().empty());
}

BOOST_AUTO_TEST_SUITE_END()
