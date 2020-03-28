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

#ifndef TOOLBOX_IO_BUFFER_HPP
#define TOOLBOX_IO_BUFFER_HPP

#include <toolbox/Config.h>

#include <boost/asio/buffer.hpp>

namespace toolbox {
inline namespace io {

using ConstBuffer = boost::asio::const_buffer;
using MutableBuffer = boost::asio::mutable_buffer;

using boost::asio::buffer_cast;
using boost::asio::buffer_size;

class TOOLBOX_API Buffer {
  public:
    explicit Buffer(std::size_t capacity) { buf_.reserve(capacity); }
    Buffer() = default;
    ~Buffer() = default;

    // Copy.
    Buffer(const Buffer&) = default;
    Buffer& operator=(const Buffer&) = default;

    // Move.
    Buffer(Buffer&&) noexcept = default;
    Buffer& operator=(Buffer&&) noexcept = default;

    /// Returns available data as a buffer.
    ConstBuffer data() const noexcept { return {rptr(), size()}; }

    /// Returns slice of available data as a buffer.
    ConstBuffer data(std::size_t limit) const noexcept { return {rptr(), std::min(limit, size())}; }

    /// Returns available data as a string view.
    std::string_view str() const noexcept { return {rptr(), size()}; }

    /// Returns slice of available data as a string view.
    std::string_view str(std::size_t limit) const noexcept
    {
        return {rptr(), std::min(limit, size())};
    }

    /// Returns true if read buffer is empty.
    bool empty() const noexcept { return size() == 0U; };

    /// Returns number of bytes available for read.
    std::size_t size() const noexcept { return wpos_ - rpos_; }

    /// Clear buffer.
    void clear() noexcept
    {
        rpos_ = wpos_ = 0;
        buf_.clear();
    }

    /// Move characters from the write sequence to the read sequence.
    void commit(std::size_t count) noexcept { wpos_ += count; }

    /// Remove characters from the read sequence.
    void consume(std::size_t count) noexcept;

    /// Returns write buffer of at least size bytes.
    MutableBuffer prepare(std::size_t size);

    /// Reserve storage.
    void reserve(std::size_t capacity) { buf_.reserve(capacity); }

  private:
    const char* rptr() const noexcept { return buf_.data() + rpos_; }
    char* wptr() noexcept { return buf_.data() + wpos_; }
    std::size_t available() const noexcept { return buf_.size() - wpos_; }

    std::size_t rpos_{}, wpos_{};
    std::vector<char> buf_;
};

TOOLBOX_API ConstBuffer advance(ConstBuffer buf, std::size_t n) noexcept;
TOOLBOX_API MutableBuffer advance(MutableBuffer buf, std::size_t n) noexcept;

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_BUFFER_HPP
