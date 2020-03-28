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

#ifndef TOOLBOX_NET_FRAME_HPP
#define TOOLBOX_NET_FRAME_HPP

#include <toolbox/io/Buffer.hpp>

#include <cassert>
#include <cstdint>
#include <string_view>

namespace toolbox {
inline namespace net {

/// Reads a binary-encoded 4 byte integer from the input buffer.
///
/// The length is encoded as a little-endian, two's complement integer.
///
/// \param buf The input buffer, which must be at least 4 bytes in length.
/// \return the decoded 4 byte integer.
constexpr std::uint32_t get_length(const char* buf) noexcept
{
    return ((buf[3] & 0xff) << 24) //
        | ((buf[2] & 0xff) << 16)  //
        | ((buf[1] & 0xff) << 8)   //
        | (buf[0] & 0xff);
}

/// Reads a binary-encoded 4 byte integer from the input buffer.
///
/// The length is encoded as a little-endian, two's complement integer.
///
/// \param buf The input buffer, which must be at least 4 bytes in length.
/// \return the decoded 4 byte integer.
inline std::uint32_t get_length(ConstBuffer buf) noexcept
{
    assert(buffer_size(buf) >= 4);
    return get_length(buffer_cast<const char*>(buf));
}

/// Writes a binary-encoded 4 byte integer to the output buffer.
///
/// The length is encoded as a little-endian, two's complement integer.
///
/// \param buf The output buffer, which must be at least 4 bytes in length.
/// \param len The length to be encoded.
constexpr void put_length(char* buf, std::uint32_t len) noexcept
{
    buf[0] = 0xff & len;
    buf[1] = 0xff & (len >> 8);
    buf[2] = 0xff & (len >> 16);
    buf[3] = 0xff & (len >> 24);
}

/// Writes a binary-encoded 4 byte integer to the output buffer.
///
/// The length is encoded as a little-endian, two's complement integer.
///
/// \param buf The output buffer, which must be at least 4 bytes in length.
/// \param len The length to be encoded.
inline void put_length(MutableBuffer buf, std::uint32_t len) noexcept
{
    assert(buffer_size(buf) >= 4);
    put_length(buffer_cast<char*>(buf), len);
}

/// Calls the function object for each message encapsulated in a length-prefixed frame.
///
/// \tparam FnT The type of the function object.
/// \param buf The input buffer.
/// \param fn The function object that is called for each complete message.
/// \return the total number of consumed bytes.
template <typename FnT>
std::size_t parse_frame(ConstBuffer buf, FnT fn)
{
    std::size_t consumed{0};
    for (;;) {
        const auto* data = buffer_cast<const char*>(buf);
        const std::size_t size = buffer_size(buf);
        if (size < sizeof(std::uint32_t)) {
            break;
        }
        const auto total = sizeof(std::uint32_t) + get_length(data);
        if (size < total) {
            break;
        }
        fn(ConstBuffer{data + sizeof(std::uint32_t), total - sizeof(std::uint32_t)});
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
/// \return the total number of consumed bytes.
template <typename FnT>
std::size_t parse_frame(std::string_view buf, FnT fn)
{
    return parse_frame(ConstBuffer{buf.data(), buf.size()}, fn);
}

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_FRAME_HPP
