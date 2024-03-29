// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Types.hpp"

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(TypesSuite)

BOOST_AUTO_TEST_CASE(TypesCase)
{
    BOOST_CHECK_EQUAL(enum_string(Status::BadRequest), "Bad Request");
    BOOST_CHECK_EQUAL(enum_string(Status::Unauthorized), "Unauthorized");
    BOOST_CHECK_EQUAL(enum_string(Status::Forbidden), "Forbidden");
    BOOST_CHECK_EQUAL(enum_string(Status::NotFound), "Not Found");
    BOOST_CHECK_EQUAL(enum_string(Status::MethodNotAllowed), "Method Not Allowed");
    BOOST_CHECK_EQUAL(enum_string(Status::InternalServerError), "Internal Server Error");
    BOOST_CHECK_EQUAL(enum_string(Status::ServiceUnavailable), "Service Unavailable");
}

BOOST_AUTO_TEST_SUITE_END()
