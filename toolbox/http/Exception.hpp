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

#ifndef TOOLBOX_HTTP_EXCEPTION_HPP
#define TOOLBOX_HTTP_EXCEPTION_HPP

#include <toolbox/http/Error.hpp>
#include <toolbox/util/Exception.hpp>

namespace toolbox {
inline namespace http {

struct TOOLBOX_API Exception : util::Exception {
    explicit Exception(Status status)
    : util::Exception{status}
    {
    }
    Exception(Status status, std::string_view what)
    : util::Exception{status, what}
    {
    }
    ~Exception() override;
};

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_EXCEPTION_HPP
