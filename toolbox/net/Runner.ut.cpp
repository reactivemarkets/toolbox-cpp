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

#include "Runner.hpp"

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/Resolver.hpp>
#include <toolbox/util/String.hpp>

#include <boost/test/unit_test.hpp>

using namespace toolbox;

BOOST_AUTO_TEST_SUITE(RunnerSuite)

BOOST_AUTO_TEST_CASE(ResolverRunnerCase)
{
    Resolver res;

    pthread_setname_np(pthread_self(), "main");
    ResolverRunner resolver_runner{res, "resolver"s};

    const auto uri1 = "tcp4://192.168.1.3:443"s;
    const auto uri2 = "tcp6://[fe80::c8bf:7d86:cbdc:bda9]:443"s;
    const auto uri3 = "unix:///tmp/foo.sock"s;
    auto future1 = res.resolve(uri1, SOCK_STREAM);
    auto future2 = res.resolve(uri2, SOCK_STREAM);
    auto future3 = res.resolve(uri3, SOCK_STREAM);

    BOOST_TEST(to_string(*future1.get()) == uri1);
    BOOST_TEST(to_string(*future2.get()) == uri2);
    BOOST_TEST(to_string(*future3.get()) == uri3);
}

BOOST_AUTO_TEST_SUITE_END()
