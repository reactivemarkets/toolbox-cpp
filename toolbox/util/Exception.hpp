// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#include <string_view>
#include <toolbox/util/Stream.hpp>

namespace toolbox {
inline namespace util {

/// Maximum error message length.
constexpr std::size_t MaxErrSize{511};

struct ErrMsg {
    OStaticStream<MaxErrSize> os_;
    operator std::string_view() const noexcept {
        return os_.str();
    }
};

template <typename ValueT>
ErrMsg& operator<<(ErrMsg& err, ValueT&& val)
{
    err.os_ << std::forward<ValueT>(val);
    return err;
}

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
    Exception(Exception&&) noexcept = default;
    Exception& operator=(Exception&&) noexcept = default;

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

namespace detail {

template <typename ExceptionT>
struct PutAsJson {
    const ExceptionT* e;
};

template <typename ExceptionT>
std::ostream& operator<<(std::ostream& os, PutAsJson<ExceptionT> val)
{
    val.e->to_json(os);
    return os;
}

template <typename ExceptionT>
struct PutWithCode {
    const ExceptionT* e;
};

template <typename ExceptionT>
std::ostream& operator<<(std::ostream& os, PutWithCode<ExceptionT> val)
{
    return os << val.e->what() << " (" << val.e->code().value() << ')';
}

} // namespace detail

template <typename ExceptionT>
auto put_as_json(const ExceptionT& e)
{
    return detail::PutAsJson<ExceptionT>{.e = &e};
}

template <typename ExceptionT>
auto put_with_code(const ExceptionT& e)
{
    return detail::PutWithCode<ExceptionT>{.e = &e};
}

/// Thread-local error message. This thread-local instance of OStaticStream can be used to format
/// error messages before throwing. Note that the OStaticStream is reset each time this function is
/// called.
TOOLBOX_API ErrMsg& err_msg() noexcept;

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_EXCEPTION_HPP
