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

#if __has_include(<boost/system/detail/error_code.ipp>)
#pragma GCC visibility push(default)
#include <boost/system/detail/error_code.ipp>
#pragma GCC visibility pop
#endif

#include <netdb.h>

namespace toolbox {
inline namespace net {
namespace {
struct GaiErrorCategory : std::error_category {
    constexpr GaiErrorCategory() noexcept = default;
    ~GaiErrorCategory() final = default;

    // Copy.
    GaiErrorCategory(const GaiErrorCategory&) = delete;
    GaiErrorCategory& operator=(const GaiErrorCategory&) = delete;

    // Move.
    GaiErrorCategory(GaiErrorCategory&&) = delete;
    GaiErrorCategory& operator=(GaiErrorCategory&&) = delete;

    const char* name() const noexcept final { return "gai"; }
    std::string message(int err) const final { return gai_strerror(err); }
};

const GaiErrorCategory gai_cat_{};
} // namespace

const std::error_category& gai_error_category() noexcept
{
    return gai_cat_;
}

std::error_code make_gai_error_code(int err) noexcept
{
    return {err, gai_cat_};
}

} // namespace net
} // namespace toolbox
