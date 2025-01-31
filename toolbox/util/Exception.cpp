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

#include "Exception.hpp"

namespace toolbox {
inline namespace util {
using namespace std;
namespace {
thread_local ErrMsg err_msg_;
} // namespace

Exception::Exception(error_code ec)
: runtime_error{ec.message()}
, ec_{ec}
{
}

Exception::Exception(int err, const error_category& ecat)
: Exception{error_code{err, ecat}}
{
}

Exception::Exception(error_code ec, std::string_view what)
: runtime_error{string{what}}
, ec_{ec}
{
}

Exception::Exception(int err, const error_category& ecat, std::string_view what)
: Exception{error_code{err, ecat}, what}
{
}

Exception::~Exception() = default;

void Exception::to_json(ostream& os, int code, const char* message)
{
    os << "{\"code\":" << code         //
       << ",\"message\":\"" << message //
       << "\"}";
}

ErrMsg& err_msg() noexcept
{
    err_msg_.os_.reset();
    return err_msg_;
}

} // namespace util
} // namespace toolbox
