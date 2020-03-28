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

#ifndef TOOLBOX_UTIL_EXCEPTION_HPP
#define TOOLBOX_UTIL_EXCEPTION_HPP

#include <toolbox/util/Stream.hpp>

#include <stdexcept>

namespace toolbox {
inline namespace util {

/// Maximum error message length.
constexpr std::size_t MaxErrSize{511};

using ErrMsg = StaticStream<MaxErrSize>;

class TOOLBOX_API Exception : public std::runtime_error {
  public:
    explicit Exception(std::error_code ec = std::error_code());
    Exception(int err, const std::error_category& ecat);
    Exception(std::error_code ec, std::string_view what);
    Exception(int err, const std::error_category& ecat, std::string_view what);
    ~Exception() override;

    // Copy.
    Exception(const Exception&) = default;
    Exception& operator=(const Exception&) = default;

    // Move.
    Exception(Exception&&) = default;
    Exception& operator=(Exception&&) = default;

    /// Format exception as a OpenAPI JSON response message.
    /// \param os The output stream.
    /// \param code The error code.
    /// \param message The error Message.
    static void to_json(std::ostream& os, int code, const char* message);

    /// Format exception as a OpenAPI JSON response message.
    /// \param os The output stream.
    /// \param code The error code.
    /// \param message The error Message.
    static void to_json(std::ostream& os, const std::error_code& code, const char* message)
    {
        to_json(os, code.value(), message);
    }

    /// Format exception as a OpenAPI JSON response message.
    /// \param os The output stream.
    void to_json(std::ostream& os) const { to_json(os, ec_, what()); }

    /// Returns the error code.
    const std::error_code& code() const noexcept { return ec_; }

  private:
    std::error_code ec_;
};

/// Thread-local error message. This thread-local instance of StaticStream can be used to format
/// error messages before throwing. Note that the StaticStream is reset each time this function is
/// called.
TOOLBOX_API ErrMsg& err_msg() noexcept;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_EXCEPTION_HPP
