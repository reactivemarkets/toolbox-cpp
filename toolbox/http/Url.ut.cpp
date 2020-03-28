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

#include "Url.hpp"

#include <toolbox/util/String.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(UrlSuite)

BOOST_AUTO_TEST_CASE(UrlCase)
{
    Url url{"http://www.reactivemarkets.com:8277/api/sess/exec?offset=3&limit5"s};
    BOOST_TEST(url.schema() == "http"s);
    BOOST_TEST(url.host() == "www.reactivemarkets.com"s);
    BOOST_TEST(url.port() == "8277"s);
    BOOST_TEST(url.path() == "/api/sess/exec"s);
    BOOST_TEST(url.query() == "offset=3&limit5"s);
    BOOST_TEST(url.fragment().empty());
    BOOST_TEST(url.user_info().empty());
}

BOOST_AUTO_TEST_CASE(UrlViewCase)
{
    const auto sv = "http://www.reactivemarkets.com:8277/api/sess/exec?offset=3&limit5"sv;
    UrlView url{sv};
    BOOST_TEST(url.schema() == "http"sv);
    BOOST_TEST(url.host() == "www.reactivemarkets.com"sv);
    BOOST_TEST(url.port() == "8277"sv);
    BOOST_TEST(url.path() == "/api/sess/exec"sv);
    BOOST_TEST(url.query() == "offset=3&limit5"sv);
    BOOST_TEST(url.fragment().empty());
    BOOST_TEST(url.user_info().empty());
}

BOOST_AUTO_TEST_SUITE_END()
