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

#include "Traits.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

double foo(short, int, long)
{
    return 0.0;
}

} // namespace

BOOST_AUTO_TEST_SUITE(TraitsSuite)

BOOST_AUTO_TEST_CASE(TraitsFreeFunCase)
{
    // Call foo to avoid the following Clang error:
    // function 'foo' is not needed and will not be emitted.
    foo(0, 0, 0);

    using Traits = FunctionTraits<decltype(&foo)>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, void>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsFunctorCase)
{
    struct Test {
        double operator()(short, int, long) { return 0.0; }
    };

    using Traits = FunctionTraits<Test>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, Test>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsConstFunctorCase)
{
    struct Test {
        double operator()(short, int, long) const { return 0.0; }
    };

    using Traits = FunctionTraits<Test>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, Test>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsConstNoexceptFunctorCase)
{
    struct Test {
        double operator()(short, int, long) const noexcept { return 0.0; }
    };

    using Traits = FunctionTraits<Test>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, Test>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsNoexceptFunctorCase)
{
    struct Test {
        double operator()(short, int, long) noexcept { return 0.0; }
    };

    using Traits = FunctionTraits<Test>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, Test>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsLambdaCase)
{
    auto fn = [](short, int, long) -> double { return 0.0; };

    using Traits = FunctionTraits<decltype(fn)>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, remove_cv_t<decltype(fn)>>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsConstLambdaCase)
{
    const auto fn = [](short, int, long) -> double { return 0.0; };

    using Traits = FunctionTraits<decltype(fn)>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, remove_const_t<decltype(fn)>>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsConstNoexceptLambdaCase)
{
    const auto fn = [](short, int, long) noexcept -> double { return 0.0; };

    using Traits = FunctionTraits<decltype(fn)>;
    using Tuple = Traits::Pack<tuple>;

    BOOST_TEST((is_same_v<Traits::ReturnType, double>));
    BOOST_TEST((is_same_v<Traits::ClassType, remove_const_t<decltype(fn)>>));
    BOOST_TEST(Traits::Arity == 3);
    BOOST_TEST((is_same_v<Traits::ArgType<0>, short>));
    BOOST_TEST((is_same_v<Traits::ArgType<1>, int>));
    BOOST_TEST((is_same_v<Traits::ArgType<2>, long>));
    BOOST_TEST(tuple_size_v<Tuple> == 3U);
}

BOOST_AUTO_TEST_CASE(TraitsMemFunCase)
{
    struct Test {
        short foo(int) { return 0; }
        long bar(double, double) { return 0; }
    };

    using FooTraits = FunctionTraits<decltype(&Test::foo)>;
    using FooTuple = FooTraits::Pack<tuple>;

    BOOST_TEST((is_same_v<FooTraits::ReturnType, short>));
    BOOST_TEST((is_same_v<FooTraits::ClassType, Test>));
    BOOST_TEST(FooTraits::Arity == 1);
    BOOST_TEST((is_same_v<FooTraits::ArgType<0>, int>));
    BOOST_TEST(tuple_size_v<FooTuple> == 1U);

    using BarTraits = FunctionTraits<decltype(&Test::bar)>;
    using BarTuple = BarTraits::Pack<tuple>;

    BOOST_TEST((is_same_v<BarTraits::ReturnType, long>));
    BOOST_TEST((is_same_v<BarTraits::ClassType, Test>));
    BOOST_TEST(BarTraits::Arity == 2);
    BOOST_TEST((is_same_v<BarTraits::ArgType<0>, double>));
    BOOST_TEST((is_same_v<BarTraits::ArgType<1>, double>));
    BOOST_TEST(tuple_size_v<BarTuple> == 2U);
}

BOOST_AUTO_TEST_CASE(TraitsConstMemFunCase)
{
    struct Test {
        short foo(int) const { return 0; }
        long bar(double, double) const { return 0; }
    };

    using FooTraits = FunctionTraits<decltype(&Test::foo)>;
    using FooTuple = FooTraits::Pack<tuple>;

    BOOST_TEST((is_same_v<FooTraits::ReturnType, short>));
    BOOST_TEST((is_same_v<FooTraits::ClassType, Test>));
    BOOST_TEST(FooTraits::Arity == 1);
    BOOST_TEST((is_same_v<FooTraits::ArgType<0>, int>));
    BOOST_TEST(tuple_size_v<FooTuple> == 1U);

    using BarTraits = FunctionTraits<decltype(&Test::bar)>;
    using BarTuple = BarTraits::Pack<tuple>;

    BOOST_TEST((is_same_v<BarTraits::ReturnType, long>));
    BOOST_TEST((is_same_v<BarTraits::ClassType, Test>));
    BOOST_TEST(BarTraits::Arity == 2);
    BOOST_TEST((is_same_v<BarTraits::ArgType<0>, double>));
    BOOST_TEST((is_same_v<BarTraits::ArgType<1>, double>));
    BOOST_TEST(tuple_size_v<BarTuple> == 2U);
}

BOOST_AUTO_TEST_SUITE_END()
