// The Reactive C++ Toolbox.
// Copyright (C) 2025 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_OSTREAM_BASE_HPP
#define TOOLBOX_UTIL_OSTREAM_BASE_HPP

#include <algorithm>
#include <charconv>
#include <concepts>
#include <type_traits>
#include <string>
#include <string_view>
#include <cassert>
#include <limits>

#include "TypeTraits.hpp"

namespace toolbox {
inline namespace util {

/*
DerivedT is required to implement the following:

1) char* do_prepare_space(std::size_t num_bytes)
    - returns writable buffer of atleast `num_bytes` bytes for immediate writing
    - will be followed by a call to `do_relinquish_space` consuming up to `num_bytes`

2) void do_relinquish_space(std::size_t consumed_num_bytes)
    - data of size `consumed_num_bytes` was written into given buffer (by `do_prepare_space`)
    - The remaining unconsumed bytes are relinquished back to derived class

3) void do_set_badbit()
    - informs derived type of failure when outputting, specifically invoked on:
        - failure to allocate space
        - failure to output
*/

template <class DerivedT>
class OStreamBase;

namespace detail {
template <class T>
concept ExcludedIntegral = is_any_of_v<T, char8_t, char16_t, char32_t, wchar_t>;

template <class T>
concept AllowedIntegral = std::is_integral_v<T> && !ExcludedIntegral<T>;

template <class T>
concept AllowedNumeric = AllowedIntegral<T> || std::floating_point<T>;

template <class T>
concept AllowedChar = is_any_of_v<T, char, signed char, unsigned char>;

template <class T>
concept InheritsBasicOStream =
    std::is_base_of_v<OStreamBase<std::remove_cvref_t<T>>, std::remove_cvref_t<T>>;

} // namespace detail

template <class DerivedT>
class OStreamBase {
  public:
    DerivedT& put_data(const char* data, std::size_t data_size);

    template <class T>
        requires detail::AllowedChar<T>
    DerivedT& put_char(T ch);

    DerivedT& put_num(bool val);

    template <class T>
        requires (detail::AllowedIntegral<T> && !std::same_as<T, bool>)
    DerivedT& put_num(T val);

    template <class T>
        requires std::floating_point<T>
    DerivedT& put_num(T val);

    /// For compatability with std::ostream API.
    DerivedT& put(char ch);
    DerivedT& write(const char* data, std::size_t sz);

  protected:
    // A number requires exactly N bytes to print, however put_num() will request a larger buffer
    // size for performance reasons (ofcourse, when the buffer is relinquished it will correctly
    // indicated that N bytes are consumed)
    // However, if derived class cannot fullfill this request, then will fallback to slower path
    // that requests exactly the N bytes required to print the number.
    static constexpr std::size_t PutNumMaxBufRequest = 32u;

  private:
    DerivedT& get_derived() noexcept;

    char* prepare_space(std::size_t num_bytes);
    void relinquish_space(std::size_t consumed_num_bytes);
    void set_badbit();
};

template <class DerivedT>
DerivedT& OStreamBase<DerivedT>::get_derived() noexcept
{
    // sizeof(T) results in a compiler error if T is an incomplete type
    static_assert(sizeof(DerivedT) >= 0, "incomplete derived type");
    static_assert(std::is_base_of_v<OStreamBase<DerivedT>, DerivedT>);
    return static_cast<DerivedT&>(*this);
}

template <class DerivedT>
char* OStreamBase<DerivedT>::prepare_space(std::size_t num_bytes)
{
    return get_derived().do_prepare_space(num_bytes);
}

template <class DerivedT>
void OStreamBase<DerivedT>::relinquish_space(std::size_t consumed_num_bytes)
{
    return get_derived().do_relinquish_space(consumed_num_bytes);
}

template <class DerivedT>
void OStreamBase<DerivedT>::set_badbit()
{
    get_derived().do_set_badbit();
}

template <class DerivedT>
DerivedT& OStreamBase<DerivedT>::put_data(const char* data, std::size_t data_size)
{
    char* buf = prepare_space(data_size);
    if (buf != nullptr) [[likely]] {
        std::copy(data, data + data_size, buf);
        relinquish_space(data_size);
    } else {
        set_badbit();
    }
    return get_derived();
}

template <class DerivedT>
template <class T> requires detail::AllowedChar<T>
DerivedT& OStreamBase<DerivedT>::put_char(T ch)
{
    char* buf = prepare_space(1);
    if (buf != nullptr) [[likely]] {
        *buf = ch;
        relinquish_space(1);
    } else {
        set_badbit();
    }
    return get_derived();
}

template <class DerivedT>
DerivedT& OStreamBase<DerivedT>::put_num(bool val)
{
    put_char(val ? '1' : '0');
    return get_derived();
}

template <class DerivedT>
template <class T> requires (detail::AllowedIntegral<T> && !std::same_as<T, bool>)
DerivedT& OStreamBase<DerivedT>::put_num(T val)
{
    constexpr std::size_t MaxBytesNeeded
        = dec_digits(std::numeric_limits<T>::max())
        + std::is_signed_v<T>;

    static_assert(MaxBytesNeeded <= PutNumMaxBufRequest);

    char* buf = prepare_space(MaxBytesNeeded);
    std::size_t buf_sz = MaxBytesNeeded;

    if (buf == nullptr) [[unlikely]] {
        // slightly slower path
        auto digits = dec_digits(val);
        if constexpr (std::is_signed_v<T>) {
            digits += (val < 0);
        }
        buf = prepare_space(digits);
        buf_sz = digits;
    }

    if (buf != nullptr) [[likely]] {
        // impossible for it to fail (N.B. to_chars is non-throwing)
        const auto [end, ec] = std::to_chars(buf, buf + buf_sz, val);
        assert(ec == std::errc());
        relinquish_space(end - buf);
    } else {
        set_badbit();
    }

    return get_derived();
}

template <class DerivedT>
template <class T> requires std::floating_point<T>
DerivedT& OStreamBase<DerivedT>::put_num(T val)
{
    char* buf = prepare_space(PutNumMaxBufRequest);
    if (buf != nullptr) [[likely]] { // fast path
        // impossible for to_chars to fail (N.B. to_chars is non-throwing)
        const auto [end, ec] = std::to_chars(buf, buf + PutNumMaxBufRequest, val);
        relinquish_space(end - buf);
        assert(ec == std::errc());
    }
    else {
        // slower path
        // impossible for to_chars to fail (N.B. to_chars is non-throwing)
        char local_space[PutNumMaxBufRequest];
        const auto [end, ec] = std::to_chars(local_space, local_space + PutNumMaxBufRequest, val);
        assert(ec == std::errc());

        std::size_t consumed = end - local_space;
        buf = prepare_space(consumed);

        if (buf != nullptr) [[likely]] {
            std::copy(local_space, end, buf);
            relinquish_space(consumed);
        } else {
            set_badbit();
        }
    }

    return get_derived();
}

template <class DerivedT>
DerivedT& OStreamBase<DerivedT>::put(char ch)
{
    return put_char(ch);
}

template <class DerivedT>
DerivedT& OStreamBase<DerivedT>::write(const char* data, std::size_t sz)
{
    return put_data(data, sz);
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, bool value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, short value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, unsigned short value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, int value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, unsigned int value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, long value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, unsigned long value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, long long value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, unsigned long long value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, float value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, double value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT&& os, long double value)
{
    os.put_num(value);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, char ch)
{
    os.put_char(ch);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, signed char ch)
{
    os.put_char(ch);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, unsigned char ch)
{
    os.put_char(ch);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, const char* s)
{
    std::size_t len = std::char_traits<char>::length(s);
    os.put_data(s, len);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, const signed char* s)
{
    const char* data = std::bit_cast<const char*>(s);
    std::size_t len = std::char_traits<char>::length(data);
    os.put_data(data, len);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, const unsigned char* s)
{
    const char* data = std::bit_cast<const char*>(s);
    std::size_t len = std::char_traits<char>::length(data);
    os.put_data(data, len);
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, const std::string& s)
{
    os.put_data(s.data(), s.size());
    return os;
}

template <class StreamT>
    requires detail::InheritsBasicOStream<StreamT>
StreamT& operator<<(StreamT& os, std::string_view s)
{
    os.put_data(s.data(), s.size());
    return os;
}

template <class StreamT, class T>
    requires (detail::InheritsBasicOStream<StreamT> &&
              std::is_rvalue_reference_v<StreamT&&>)
StreamT&& operator<<(StreamT&& os, const T& value)
{
    os << value;
    return static_cast<StreamT&&>(os);
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_OSTREAM_BASE_HPP
