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

#ifndef TOOLBOX_UTIL_RINGBUFFER_HPP
#define TOOLBOX_UTIL_RINGBUFFER_HPP

#include <toolbox/util/Math.hpp>

#include <memory>

namespace toolbox {
inline namespace util {

template <typename ValueT>
class RingBuffer {
  public:
    explicit RingBuffer(std::size_t capacity)
    : capacity_{next_pow2(capacity)}
    , mask_{capacity_ - 1}
    , buf_{new ValueT[capacity_]}
    {
    }
    ~RingBuffer() = default;

    // Copy.
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer& operator=(const RingBuffer&) = delete;

    // Move.
    RingBuffer(RingBuffer&&) = default;
    RingBuffer& operator=(RingBuffer&&) = default;

    /// Returns true if the container is empty.
    bool empty() const noexcept { return rpos_ == wpos_; }
    /// Returns true if the container is full.
    bool full() const noexcept { return size() == capacity_; }
    /// Returns the number of available elements.
    std::size_t available() const noexcept { return capacity_ - size(); }
    /// Returns the maximum number of elements the container can hold.
    /// I.e. the container's capacity.
    std::size_t capacity() const noexcept { return wpos_ - rpos_; }
    /// Returns the maximum number of elements the container can hold.
    /// I.e. the container's capacity.
    std::size_t max_size() const noexcept { return capacity(); }
    /// Returns true if the number of elements in the container.
    std::size_t size() const noexcept { return wpos_ - rpos_; }
    const ValueT& front() const noexcept { return buf_[rpos_ & mask_]; }
    const ValueT& back() const noexcept { return buf_[(wpos_ - 1) & mask_]; }

    ValueT& front() noexcept { return buf_[rpos_ & mask_]; }
    ValueT& back() noexcept { return buf_[(wpos_ - 1) & mask_]; }
    void clear() noexcept { rpos_ = wpos_ = 0; }
    void pop() noexcept { ++rpos_; }
    void push(const ValueT& val)
    {
        auto& ref = buf_[wpos_ & mask_];
        ref = val;
        if (full()) {
            ++rpos_;
        }
        ++wpos_;
    }
    template <typename FnT>
    void fetch(FnT fn)
    {
        const auto& ref = buf_[rpos_ & mask_];
        fn(ref);
        ++rpos_;
    }
    template <typename FnT>
    void write(FnT fn)
    {
        auto& ref = buf_[wpos_ & mask_];
        fn(ref);
        if (full()) {
            ++rpos_;
        }
        ++wpos_;
    }

  private:
    // Ensure that read and write positions are in different cache-lines.
    struct alignas(64) {
        std::size_t capacity_;
        std::size_t mask_;
        uint64_t rpos_{0};
    };
    uint64_t wpos_{0};
    std::unique_ptr<ValueT[]> buf_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_RINGBUFFER_HPP
