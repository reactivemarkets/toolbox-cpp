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

#include "Argv.hpp"

#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace std;
using namespace toolbox;

namespace {

struct Opts {
    vector<string_view> args, fruit, veg;
    bool blend{false};
};

void get_opts(int argc, const char* const argv[], Opts& opts)
{
    ArgvLexer lex{argc, argv};
    while (!lex.empty()) {
        const auto opt = lex.opt();
        if (opt.empty()) {
            opts.args.push_back(lex.pop_value());
        } else if (opt == "b" || opt == "blend") {
            opts.blend = true;
            lex.pop_switch();
        } else if (opt == "f" || opt == "fruit") {
            opts.fruit.push_back(lex.pop_value());
        } else if (opt == "v" || opt == "veg") {
            opts.veg.push_back(lex.pop_value());
        } else {
            throw runtime_error{"unknown option: " + string{opt}};
        }
    }
}

template <size_t N>
using Argv = array<const char*, N>;

} // namespace

BOOST_AUTO_TEST_SUITE(ArgvSuite)

BOOST_AUTO_TEST_CASE(ArgvEmptyCase)
{
    Opts opts;
    Argv<0> argv{};
    get_opts(argv.size(), argv.data(), opts);

    BOOST_TEST(opts.args.empty());
    BOOST_TEST(opts.fruit.empty());
    BOOST_TEST(opts.veg.empty());
    BOOST_TEST(!opts.blend);
}

BOOST_AUTO_TEST_CASE(ArgvPositionalCase)
{
    Opts opts;
    Argv<5> argv{"foo", "-", "bar", "--", "baz"};
    get_opts(argv.size(), argv.data(), opts);

    BOOST_TEST(opts.args.size() == 5);
    BOOST_TEST(opts.args.at(0) == "foo"sv);
    BOOST_TEST(opts.args.at(1) == "-"sv);
    BOOST_TEST(opts.args.at(2) == "bar"sv);
    BOOST_TEST(opts.args.at(3) == "--"sv);
    BOOST_TEST(opts.args.at(4) == "baz"sv);
    BOOST_TEST(opts.fruit.empty());
    BOOST_TEST(opts.veg.empty());
    BOOST_TEST(!opts.blend);
}

BOOST_AUTO_TEST_CASE(ArgvShortOptCase)
{
    {
        Opts opts;
        Argv<2> argv{"-fapple", "-vcarrot"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
    {
        Opts opts;
        Argv<4> argv{"-v", "carrot", "-f", "apple"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
    {
        // Empty option value.
        Opts opts;
        Argv<4> argv{"-f", "", "-v", ""};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0).empty());
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0).empty());
        BOOST_TEST(!opts.blend);
    }
    {
        // Missing option value.
        Opts opts;
        Argv<1> argv{"-v"};
        BOOST_CHECK_THROW(get_opts(argv.size(), argv.data(), opts), std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(ArgvMultiShortOptCase)
{
    {
        Opts opts;
        Argv<2> argv{"-fapple", "-fbanana"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 2);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.fruit.at(1) == "banana"sv);
        BOOST_TEST(opts.veg.empty());
        BOOST_TEST(!opts.blend);
    }
    {
        Opts opts;
        Argv<4> argv{"-v", "broccoli", "-v", "carrot"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.empty());
        BOOST_TEST(opts.veg.size() == 2);
        BOOST_TEST(opts.veg.at(0) == "broccoli"sv);
        BOOST_TEST(opts.veg.at(1) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
}

BOOST_AUTO_TEST_CASE(ArgvLongOptCase)
{
    {
        Opts opts;
        Argv<2> argv{"--fruit=apple", "--veg=carrot"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
    {
        Opts opts;
        Argv<4> argv{"--veg", "carrot", "--fruit", "apple"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
    {
        // Empty option value.
        Opts opts;
        Argv<2> argv{"--fruit=", "--veg="};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 1);
        BOOST_TEST(opts.fruit.at(0).empty());
        BOOST_TEST(opts.veg.size() == 1);
        BOOST_TEST(opts.veg.at(0).empty());
        BOOST_TEST(!opts.blend);
    }
    {
        // Missing option value.
        Opts opts;
        Argv<1> argv{"--veg"};
        BOOST_CHECK_THROW(get_opts(argv.size(), argv.data(), opts), std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(ArgvMultiLongOptCase)
{
    {
        Opts opts;
        Argv<2> argv{"--fruit=apple", "--fruit=banana"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.size() == 2);
        BOOST_TEST(opts.fruit.at(0) == "apple"sv);
        BOOST_TEST(opts.fruit.at(1) == "banana"sv);
        BOOST_TEST(opts.veg.empty());
        BOOST_TEST(!opts.blend);
    }
    {
        Opts opts;
        Argv<4> argv{"--veg", "broccoli", "--veg", "carrot"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.empty());
        BOOST_TEST(opts.veg.size() == 2);
        BOOST_TEST(opts.veg.at(0) == "broccoli"sv);
        BOOST_TEST(opts.veg.at(1) == "carrot"sv);
        BOOST_TEST(!opts.blend);
    }
}

BOOST_AUTO_TEST_CASE(ArgvSwitchCase)
{
    {
        Opts opts;
        Argv<1> argv{"-b"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.empty());
        BOOST_TEST(opts.veg.empty());
        BOOST_TEST(opts.blend);
    }
    {
        Opts opts;
        Argv<1> argv{"--blend"};
        get_opts(argv.size(), argv.data(), opts);

        BOOST_TEST(opts.args.empty());
        BOOST_TEST(opts.fruit.empty());
        BOOST_TEST(opts.veg.empty());
        BOOST_TEST(opts.blend);
    }
    {
        // Unexpected switch value.
        Opts opts;
        Argv<1> argv{"-b123"};
        BOOST_CHECK_THROW(get_opts(argv.size(), argv.data(), opts), std::runtime_error);
    }
    {
        // Unexpected switch value.
        Opts opts;
        Argv<1> argv{"--blend=123"};
        BOOST_CHECK_THROW(get_opts(argv.size(), argv.data(), opts), std::runtime_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()
