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

#include "Options.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(OptionsSuite)

BOOST_AUTO_TEST_CASE(OptionsNormalCase)
{
    int var{-1};
    bool flag{false};
    std::string command;
    int factor{-1};

    Options opts{"Unit Test options [OPTIONS] [COMMAND]"};
    // clang-format off
    opts('o', "option", Value{var}.default_value(3).required(), "Option Description")
        ('s', NoOp{}, "ShortOption Description")
        ("long_opt", NoOp{}, "LongOption Description")
        ('x', Switch{flag}, "Switch Description")
        (Value{command}.default_value("init").required(), "Positional Command")
        (Value{factor}.required(), "Positional Command");
    // clang-format on

    BOOST_CHECK_EQUAL(var, 3);
    BOOST_CHECK_EQUAL(command, "init");

    stringstream ss;
    ss << opts;
    const string expected{"Usage: Unit Test options [OPTIONS] [COMMAND]\n"
                          "Options:\n"
                          "  -o, --option   Option Description\n"
                          "  -s             ShortOption Description\n"
                          "  --long_opt     LongOption Description\n"
                          "  -x             Switch Description\n"};
    BOOST_CHECK_EQUAL(ss.str(), expected);

    {
        const char* argv[] = {"executable_name", "-o", "123", "print", "456"};
        int argc = 5;

        opts.parse(argc, argv);
        BOOST_CHECK_EQUAL(var, 123);
        BOOST_CHECK_EQUAL(flag, false);
        BOOST_CHECK_EQUAL(command, "print");
        BOOST_CHECK_EQUAL(factor, 456);
    }
    {
        const char* argv[] = {"executable_name", "-x"};
        int argc = 2;

        opts.parse(argc, argv);
        BOOST_CHECK_EQUAL(flag, true);
    }
}

BOOST_AUTO_TEST_CASE(OptionsExceptionsCase)
{
    int var{-1};

    Options opts{"Exception Test options"};
    opts('l', "long_option", Value{var}, "LongOption Description");

    // Trying to add a duplicate option fails
    BOOST_CHECK_THROW(opts('x', "long_option", Value{var}, "Description"), std::runtime_error);
    BOOST_CHECK_THROW(opts('l', "l_exists", Value{var}, "Description"), std::runtime_error);
    BOOST_CHECK_THROW(opts('l', "long_option", Value{var}, "Description"), std::runtime_error);
}

BOOST_AUTO_TEST_CASE(OptionsInvalidOptionCase)
{
    {
        int var{-1};
        Options opts{"Options"};
        opts('l', "long_option", Value{var}, "LongOption Description");

        const char* argv[] = {"executable_name", "--bad", "123"};
        int argc = 3;

        BOOST_CHECK_THROW(opts.parse(argc, argv), std::runtime_error);
    }

    {
        int var{-1};
        Options opts{"Options"};
        opts('l', "long_option", Value{var}, "LongOption Description");

        const char* argv[] = {"executable_name", "-l123", "456"};
        int argc = 3;

        BOOST_CHECK_THROW(opts.parse(argc, argv), std::runtime_error);
    }
}

BOOST_AUTO_TEST_CASE(OptionsNoValueCase)
{
    int var{-1};
    Options opts{"Options"};
    opts('l', "long_option", Value{var}, "LongOption Description");

    const char* argv[] = {"executable_name", "-l"};
    int argc = 2;

    BOOST_CHECK_THROW(opts.parse(argc, argv);, std::runtime_error);
}

BOOST_AUTO_TEST_CASE(OptionsMultiToken)
{
    int var{-1};
    std::vector<int> single;
    std::vector<int> multiple;

    {
        Options opts{"Multitoken Test options"};
        opts('v', Value{var}, "Raw int")('s', Value{single}, "Single int")(
            'm', Value{multiple}.multitoken(), "Multiple ints");

        const char* argv[] = {"executable_name", "-v", "1", "-s", "2", "-m", "3", "-m", "4"};
        int argc = 9;

        opts.parse(argc, argv);
        BOOST_CHECK_EQUAL(var, 1);
        BOOST_CHECK_EQUAL(single[0], 2);
        BOOST_CHECK_EQUAL(multiple[0], 3);
        BOOST_CHECK_EQUAL(multiple[1], 4);
    }

    {
        Options opts{"Multitoken Test options"};
        opts('v', Value{var}, "Raw int")('s', Value{single}, "Single int")(
            'm', Value{multiple}.multitoken(), "Multiple ints");

        const char* argv[] = {"executable_name", "-s", "1", "-s", "2"};
        int argc = 5;

        BOOST_CHECK_THROW(opts.parse(argc, argv), std::runtime_error);
    }
}

BOOST_AUTO_TEST_SUITE_END()
