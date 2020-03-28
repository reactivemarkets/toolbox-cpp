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

#ifndef TOOLBOX_HTTP_ERROR_HPP
#define TOOLBOX_HTTP_ERROR_HPP

#include <toolbox/http/Types.hpp>

#include <system_error>

namespace toolbox {
inline namespace http {

TOOLBOX_API HttpStatus http_status(const std::error_code& ec);
TOOLBOX_API const std::error_category& http_error_category() noexcept;
TOOLBOX_API std::error_code make_error_code(HttpStatus err);

} // namespace http
} // namespace toolbox

namespace std {
template <>
struct is_error_code_enum<toolbox::http::HttpStatus> : true_type {
};
} // namespace std

#endif // TOOLBOX_HTTP_ERROR_HPP
