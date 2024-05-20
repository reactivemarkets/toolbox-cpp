// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2023 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_STRING_HPP
#define TOOLBOX_UTIL_STRING_HPP

#include <toolbox/util/Concepts.hpp>
#include <toolbox/util/TypeTraits.hpp>

#include <charconv>
#include <cstring>
#include <sstream>

namespace toolbox {
inline namespace util {

template <typename ValueT>
constexpr ValueT from_string(std::string_view sv)
{
    using Traits = TypeTraits<ValueT>;
    return Traits::from_string(sv);
}
static_assert(from_string<int>(std::string_view{"-123"}) == -123);

template <typename ValueT>
constexpr ValueT from_string(const std::string& s)
{
    using Traits = TypeTraits<ValueT>;
    return Traits::from_string(s);
}

template <typename ValueT>
constexpr ValueT from_string(const char* s)
{
    using Traits = TypeTraits<ValueT>;
    return Traits::from_string(std::string_view{s});
}
static_assert(from_string<int>("-123") == -123);

template <typename ValueT>
std::string to_string(ValueT&& val)
{
    std::stringstream ss;
    ss << val;
    return ss.str();
}

template <typename ValueT>
// clang-format off
requires Arithmetic<ValueT>
std::string to_string(ValueT&& val)
// clang-format on
{
    return std::to_string(val);
}

template <std::size_t SizeN>
constexpr std::string_view to_string_view(const char (&val)[SizeN]) noexcept
{
    return {val, strnlen(val, SizeN)};
}

TOOLBOX_API void ltrim(std::string_view& s) noexcept;

TOOLBOX_API void ltrim(std::string& s) noexcept;

TOOLBOX_API void rtrim(std::string_view& s) noexcept;

TOOLBOX_API void rtrim(std::string& s) noexcept;

inline void trim(std::string_view& s) noexcept
{
    ltrim(s);
    rtrim(s);
}

inline void trim(std::string& s) noexcept
{
    ltrim(s);
    rtrim(s);
}

inline std::string_view ltrim_copy(std::string_view s) noexcept
{
    ltrim(s);
    return s;
}

inline std::string ltrim_copy(std::string s) noexcept
{
    ltrim(s);
    return s;
}

inline std::string_view rtrim_copy(std::string_view s) noexcept
{
    rtrim(s);
    return s;
}

inline std::string rtrim_copy(std::string s) noexcept
{
    rtrim(s);
    return s;
}

inline std::string_view trim_copy(std::string_view s) noexcept
{
    trim(s);
    return s;
}

inline std::string trim_copy(std::string s) noexcept
{
    trim(s);
    return s;
}

TOOLBOX_API std::pair<std::string_view, std::string_view> split_pair(std::string_view s,
                                                                     char delim) noexcept;

TOOLBOX_API std::pair<std::string, std::string> split_pair(const std::string& s, char delim);

/// Returns the length of right-padded string.
/// \tparam PadC The character used for padding.
/// \param src The source string.
/// \param n The maximum size of the source string.
/// \return the length of src if less than n, otherwise n.
template <char PadC>
constexpr std::size_t pstrlen(const char* src, std::size_t n) noexcept
{
    if constexpr (PadC == '\0') {
        // Optimised case.
        return strnlen(src, n);
    } else {
        std::size_t i{0};
        while (i < n && src[i] != PadC) {
            ++i;
        }
        return i;
    }
}

/// Returns the length of right-padded string.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the source string.
/// \param src The source string.
/// \return the length of src.
template <char PadC, std::size_t SizeN>
constexpr std::size_t pstrlen(const char (&src)[SizeN]) noexcept
{
    return pstrlen<PadC>(src, SizeN);
}

/// Copy src string to right-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \param dst The destination buffer.
/// \param src The source string.
/// \param n The maximum size of the destination buffer.
/// \return the number of bytes copied to the destination buffer.
template <char PadC>
constexpr std::size_t pstrcpy(char* dst, const char* src, std::size_t n) noexcept
{
    if constexpr (PadC == '\0') {
#ifndef __clang__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        // Optimised case.
        return stpncpy(dst, src, n) - dst;
#ifndef __clang__
#pragma GCC diagnostic pop
#endif
    } else {
        std::size_t i{0};
        for (; i < n && src[i] != '\0'; ++i) {
            dst[i] = src[i];
        }
        if (i < n) {
            std::memset(dst + i, PadC, n - i);
        }
        return i;
    }
}

/// Copy src string to right-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the destination buffer.
/// \param dst The destination buffer.
/// \param src The source string.
/// \return the number of bytes copied to the destination buffer.
template <char PadC, std::size_t SizeN>
constexpr std::size_t pstrcpy(char (&dst)[SizeN], const char* src) noexcept
{
    return pstrcpy<PadC>(dst, src, SizeN);
}

/// Copy src string to right-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \param dst The destination buffer.
/// \param src The source string.
/// \param n The maximum size of the destination buffer.
/// \return the number of bytes copied to the destination buffer.
template <char PadC>
constexpr std::size_t pstrcpy(char* dst, std::string_view src, std::size_t n) noexcept
{
    const std::size_t len{std::min(n, src.size())};
    if (len > 0) {
        std::memcpy(dst, src.data(), len);
    }
    if (len < n) {
        std::memset(dst + len, PadC, n - len);
    }
    return len;
}

/// Copy src string to right-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the destination buffer.
/// \param dst The destination buffer.
/// \param src The source string.
/// \return the number of bytes copied to the destination buffer.
template <char PadC, std::size_t SizeN>
constexpr std::size_t pstrcpy(char (&dst)[SizeN], std::string_view src) noexcept
{
    return pstrcpy<PadC>(dst, src, SizeN);
}

template <char PadC>
constexpr std::size_t pstrcpyid(char* dst, std::int64_t id, std::size_t n) noexcept
{
    auto* end = dst + n;
    const auto [eptr, ec] = std::to_chars(dst, end, id);
    if (ec == std::errc::value_too_large) {
        return 0;
    }
    if (eptr < end) {
        std::memset(eptr, PadC, end - eptr);
    }
    return eptr - dst;
}

template <char PadC, std::size_t SizeN>
constexpr std::size_t pstrcpyid(char (&dst)[SizeN], std::int64_t id) noexcept
{
    return pstrcpyid<PadC>(dst, id, SizeN);
}

/// Returns the length of left-padded string.
/// \tparam PadC The character used for padding.
/// \param src The source string.
/// \param n The maximum size of the source string.
/// \return the length of src if less than n, otherwise n.
template <char PadC>
constexpr std::size_t lpstrlen(const char* src, std::size_t n) noexcept
{
    std::size_t i{0};
    while (i < n && src[i] == PadC) {
        ++i;
    }
    return n - i;
}

/// Returns the length of left-padded string.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the source string.
/// \param src The source string.
/// \return the length of src.
template <char PadC, std::size_t SizeN>
constexpr std::size_t lpstrlen(const char (&src)[SizeN]) noexcept
{
    return lpstrlen<PadC>(src, SizeN);
}

/// Copy src string to left-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \param dst The destination buffer.
/// \param src The source string.
/// \param n The maximum size of the destination buffer.
/// \return the number of bytes copied to the destination buffer.
template <char PadC>
constexpr std::size_t lpstrcpy(char* dst, const char* src, std::size_t n) noexcept
{
    const std::size_t len{strnlen(src, n)};
    if (len < n) {
        std::memset(dst, PadC, n - len);
    }
    if (len > 0) {
        std::memcpy(dst + (n - len), src, len);
    }
    return len;
}

/// Copy src string to left-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the destination buffer.
/// \param dst The destination buffer.
/// \param src The source string.
/// \return the number of bytes copied to the destination buffer.
template <char PadC, std::size_t SizeN>
constexpr std::size_t lpstrcpy(char (&dst)[SizeN], const char* src) noexcept
{
    return lpstrcpy<PadC>(dst, src, SizeN);
}

/// Copy src string to left-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \param dst The destination buffer.
/// \param src The source string.
/// \param n The maximum size of the destination buffer.
/// \return the number of bytes copied to the destination buffer.
template <char PadC>
constexpr std::size_t lpstrcpy(char* dst, std::string_view src, std::size_t n) noexcept
{
    const std::size_t len{std::min(n, src.size())};
    if (len < n) {
        std::memset(dst, PadC, n - len);
    }
    if (len > 0) {
        std::memcpy(dst + (n - len), src.data(), len);
    }
    return len;
}

/// Copy src string to left-padded buffer dst.
/// \tparam PadC The character used for padding.
/// \tparam SizeN The maximum size of the destination buffer.
/// \param dst The destination buffer.
/// \param src The source string.
/// \return the number of bytes copied to the destination buffer.
template <char PadC, std::size_t SizeN>
constexpr std::size_t lpstrcpy(char (&dst)[SizeN], std::string_view src) noexcept
{
    return lpstrcpy<PadC>(dst, src, SizeN);
}

template <typename... ArgsT>
std::string make_string(ArgsT&&... args)
{
    std::stringstream os;
    (os << ... << args);
    return os.str();
}
} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STRING_HPP
