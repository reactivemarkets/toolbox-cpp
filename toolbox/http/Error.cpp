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

#include "Error.hpp"

namespace toolbox {
using namespace std::literals::string_literals;
inline namespace http {
namespace {
struct HttpErrorCategory : std::error_category {
    constexpr HttpErrorCategory() noexcept = default;
    ~HttpErrorCategory() final = default;

    // Copy.
    HttpErrorCategory(const HttpErrorCategory&) = delete;
    HttpErrorCategory& operator=(const HttpErrorCategory&) = delete;

    // Move.
    HttpErrorCategory(HttpErrorCategory&&) = delete;
    HttpErrorCategory& operator=(HttpErrorCategory&&) = delete;

    const char* name() const noexcept final { return "http"; }
    std::string message(int err) const final { return enum_string(static_cast<HttpStatus>(err)); }
};

const HttpErrorCategory ecat_{};
} // namespace

const std::error_category& http_error_category() noexcept
{
    return ecat_;
}

std::error_code make_error_code(HttpStatus status)
{
    return {static_cast<int>(status), ecat_};
}

HttpStatus http_status(const std::error_code& ec)
{
    if (ec.category() != http_error_category()) {
        return HttpStatus::InternalServerError;
    }
    return static_cast<HttpStatus>(ec.value());
}

} // namespace http
} // namespace toolbox
