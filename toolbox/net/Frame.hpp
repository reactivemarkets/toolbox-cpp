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

#ifndef TOOLBOX_NET_FRAME_HPP
#define TOOLBOX_NET_FRAME_HPP

#include <toolbox/io/Buffer.hpp>

#include <cassert>

namespace toolbox {
inline namespace net {
namespace detail {

/// Reads a binary-encoded 2 byte integer from the input buffer.
/// N.B. the Native template parameter is provided for testability.
///
/// \tparam Native Endianness of native/target platform.
/// \param buf The input buffer, which must be at least 2 bytes in length.
/// \param net_byte_order Endianness used for network byte order decoding.
/// \return the decoded 2 byte integer.
template <std::endian Native = std::endian::native>
constexpr std::uint16_t get_length(const char* buf, std::endian net_byte_order) noexcept
{
    // If byte swap required then 1, else 0.
    // These values provide indices into buffer, which is assumed to be at least 2 bytes long.
    const int bswap{Native != net_byte_order};
    return ((buf[!bswap] & 0xff) << 8) | (buf[bswap] & 0xff);
}

/// Writes a binary-encoded 2 byte integer to the output buffer.
/// N.B. the Native template parameter is provided for testability.
///
/// \tparam Native Endianness of native/target platform.
/// \param buf The output buffer, which must be at least 2 bytes in length.
/// \param len The length to be encoded.
/// \param net_byte_order Endianness used for network byte order decoding.
template <std::endian Native = std::endian::native>
inline void put_length(char* buf, std::uint16_t len, std::endian net_byte_order) noexcept
{
    // If byte swap required then 1, else 0.
    // These values provide indices into buffer, which is assumed to be at least 2 bytes long.
    const int bswap{Native != net_byte_order};
    buf[bswap] = 0xff & len;
    buf[!bswap] = 0xff & (len >> 8);
}

} // namespace detail

/// Reads a binary-encoded 2 byte integer from the input buffer.
///
/// \param buf The input buffer, which must be at least 2 bytes in length.
/// \param net_byte_order Endianness used for network byte order decoding.
/// \return the decoded 2 byte integer.
constexpr std::uint16_t get_length(const char* buf, std::endian net_byte_order) noexcept
{
    return detail::get_length<>(buf, net_byte_order);
}

/// Reads a binary-encoded 2 byte integer from the input buffer.
///
/// \param buf The input buffer, which must be at least 2 bytes in length.
/// \param net_byte_order Endianness used for network byte order decoding.
/// \return the decoded 2 byte integer.
inline std::uint16_t get_length(ConstBuffer buf, std::endian net_byte_order) noexcept
{
    assert(buffer_size(buf) >= 2);
    return get_length(static_cast<const char*>(buf.data()), net_byte_order);
}

/// Writes a binary-encoded 2 byte integer to the output buffer.
///
/// \param buf The output buffer, which must be at least 2 bytes in length.
/// \param len The length to be encoded.
/// \param net_byte_order Endianness used for network byte order encoding.
inline void put_length(char* buf, std::uint16_t len, std::endian net_byte_order) noexcept
{
    detail::put_length<>(buf, len, net_byte_order);
}

/// Writes a binary-encoded 2 byte integer to the output buffer.
///
/// \param buf The output buffer, which must be at least 2 bytes in length.
/// \param len The length to be encoded.
/// \param net_byte_order Endianness used for network byte order encoding.
inline void put_length(MutableBuffer buf, std::uint16_t len, std::endian net_byte_order) noexcept
{
    assert(buffer_size(buf) >= 2);
    put_length(static_cast<char*>(buf.data()), len, net_byte_order);
}

/// Calls the function object for each message encapsulated in a length-prefixed frame.
///
/// \tparam FnT The type of the function object.
/// \param buf The input buffer.
/// \param fn The function object that is called for each complete message.
/// \param net_byte_order Endianness used for network byte order decoding.
/// \return the total number of consumed bytes.
template <typename FnT>
std::size_t parse_frame(ConstBuffer buf, FnT fn, std::endian net_byte_order)
{
    std::size_t consumed{0};
    for (;;) {
        const auto* data = static_cast<const char*>(buf.data());
        const std::size_t size = buffer_size(buf);
        if (size < sizeof(std::uint16_t)) {
            break;
        }
        const auto total = get_length(data, net_byte_order);
        if (size < total) {
            break;
        }
        fn(ConstBuffer{data + sizeof(std::uint16_t), total - sizeof(std::uint16_t)});
        buf = advance(buf, total);
        consumed += total;
    }
    return consumed;
}

/// Calls the function object for each message encapsulated in a length-prefixed frame.
///
/// \tparam FnT The type of the function object.
/// \param buf The input buffer.
/// \param fn The function object that is called for each complete message.
/// \param net_byte_order Endianness used for network byte order decoding.
/// \return the total number of consumed bytes.
template <typename FnT>
std::size_t parse_frame(std::string_view buf, FnT fn, std::endian net_byte_order)
{
    return parse_frame(ConstBuffer{buf.data(), buf.size()}, fn, net_byte_order);
}

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_FRAME_HPP
