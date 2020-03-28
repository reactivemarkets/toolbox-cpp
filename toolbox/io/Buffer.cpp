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

#include "Buffer.hpp"

namespace toolbox {
inline namespace io {

void Buffer::consume(std::size_t count) noexcept
{
    enum { ShrinkThreshold = 1024 };

    if (count == 0) {
        return;
    }
    rpos_ += count;

    // Shrink if the block of unused bytes at the front of the buffer satisfies the following
    // requirements: 1) greater than or equal to ShrinkThreshold, 1) greater than or equal to half
    // the total buffer size.
    if (rpos_ >= ShrinkThreshold && rpos_ >= (buf_.size() >> 1)) {

        // Then move unread portion to front.
        const auto n = size();
        if (n > 0) {
            // Move data to front.
            std::memmove(buf_.data(), rptr(), n);
        }

        rpos_ = 0;
        wpos_ = n;
    }
}

MutableBuffer Buffer::prepare(std::size_t size)
{
    auto avail = available();
    if (size > avail) {
        // More buffer space required.
        const auto diff = size - avail;
        buf_.resize(buf_.size() + diff);
        avail = size;
    }
    return {wptr(), avail};
}

ConstBuffer advance(ConstBuffer buf, std::size_t n) noexcept
{
    const auto* const data = buffer_cast<const char*>(buf);
    const std::size_t size = buffer_size(buf);
    const auto offset = std::min(n, size);
    return {data + offset, size - offset};
}

MutableBuffer advance(MutableBuffer buf, std::size_t n) noexcept
{
    auto* const data = buffer_cast<char*>(buf);
    const std::size_t size = buffer_size(buf);
    const auto offset = std::min(n, size);
    return {data + offset, size - offset};
}

} // namespace io
} // namespace toolbox
