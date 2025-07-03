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

#ifndef TOOLBOX_UTIL_STREAM_HPP
#define TOOLBOX_UTIL_STREAM_HPP

#include <toolbox/util/OStreamBase.hpp>
#include <toolbox/util/Concepts.hpp>
#include <toolbox/util/Storage.hpp>

namespace toolbox {
inline namespace util {

namespace detail {
struct ResetState {};
TOOLBOX_API std::ostream& operator<<(std::ostream& os, ResetState) noexcept;
} // namespace detail

/// I/O manipulator that resets I/O state.
/// Only for Streams inheriting from std::ostream.
constexpr detail::ResetState reset_state{};

/// OStream uses a dynamic storage acquired from the custom allocator.
template <std::size_t MaxN>
class OStream final : public OStreamBase<OStream<MaxN>> {
  public:
    /// Constructor for initialising the StreamBuf with a null buffer.
    explicit OStream(std::nullptr_t) noexcept
    : storage_(nullptr)
    {
    }

    /// Constructor for initialising the StreamBuf with pre-allocated storage.
    explicit OStream(StoragePtr<MaxN> storage) noexcept
    : storage_(std::move(storage))
    {
    }

    /// Default constructor allocates required storage
    OStream() : storage_(make_storage()) {}
    ~OStream() = default;

    // Copy.
    OStream(const OStream&) = delete;
    OStream& operator=(const OStream&) = delete;

    // Move.
    OStream(OStream&&) = delete;
    OStream& operator=(OStream&&) = delete;

    static StoragePtr<MaxN> make_storage() { return util::make_storage<MaxN>(); }
    const char* data() const noexcept { return storage_.get(); }
    bool empty() const noexcept { return bytes_written_ == 0u; }
    std::size_t size() const noexcept { return bytes_written_; }
    std::string_view str() const noexcept { return std::string_view{data(), size()}; }

    // returns false if overflowed or output error.
    // return true otherwise.
    explicit operator bool() const noexcept {
        return !badbit_;
    }

    /// Release the managed storage.
    StoragePtr<MaxN> release_storage() noexcept
    {
        StoragePtr<MaxN> storage;
        storage_.swap(storage);
        reset();
        return storage;
    }

    /// Update the internal storage and clear i/o state.
    void set_storage(StoragePtr<MaxN> storage) noexcept
    {
        storage_ = std::move(storage);
        reset();
    }

    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept
    {
        bytes_written_ = 0u;
        badbit_ = false;
    }

  private:
    friend OStreamBase<OStream<MaxN>>;

    std::size_t available() noexcept
    {
        if (storage_ != nullptr) [[likely]] {
            return MaxN - bytes_written_;
        } else {
            return 0u;
        }
    }

    char* wptr() noexcept
    {
        assert(storage_ != nullptr);
        void* buf_base = storage_.get();
        return static_cast<char*>(buf_base) + bytes_written_;
    }

    // Required by CRTP base (BasicOStream)
    char* do_prepare_space(std::size_t num_bytes)
    {
        if (num_bytes <= available()) [[likely]] {
            return wptr();
        } else {
            return nullptr;
        }
    }

    // Required by CRTP base (BasicOStream)
    void do_relinquish_space(std::size_t consumed_num_bytes)
    {
        bytes_written_ += consumed_num_bytes;
        assert(bytes_written_ <= MaxN);
    }

    // Required by CRTP base (BasicOStream)
    void do_set_badbit()
    {
        badbit_ = true;
    }

    StoragePtr<MaxN> storage_;
    std::size_t bytes_written_{0u};
    bool badbit_{false};
};

template <std::size_t MaxN>
class OStaticStream final : public OStreamBase<OStaticStream<MaxN>> {
  public:
    OStaticStream() = default;
    ~OStaticStream() = default;

    // Copy.
    OStaticStream(const OStaticStream&) = delete;
    OStaticStream& operator=(const OStaticStream&) = delete;

    // Move.
    OStaticStream(OStaticStream&&) = delete;
    OStaticStream& operator=(OStaticStream&&) = delete;

    const char* data() const noexcept { return buf_; }
    bool empty() const noexcept { return bytes_written_ == 0u; }
    std::size_t size() const noexcept { return std::min(bytes_written_, MaxN); }
    std::string_view str() const noexcept { return std::string_view{buf_, size()}; }

    // returns false if overflowed or output error.
    // return true otherwise.
    explicit operator bool() const {
        return !badbit_;
    }

    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept
    {
        bytes_written_ = 0;
        badbit_ = false;
    };

  private:
    friend OStreamBase<OStaticStream<MaxN>>;

    std::size_t available() noexcept
    {
        return BufSize - bytes_written_;
    }

    char* wptr() noexcept
    {
        return buf_ + bytes_written_;
    }

    // Required by CRTP base (BasicOStream)
    char* do_prepare_space(std::size_t num_bytes)
    {
        if (num_bytes <= available()) [[likely]] {
            return wptr();
        } else {
            return nullptr;
        }
    }

    // Required by CRTP base (BasicOStream)
    void do_relinquish_space(std::size_t consumed_num_bytes)
    {
        std::size_t new_written_count = bytes_written_ + consumed_num_bytes;
        if (new_written_count > MaxN) [[unlikely]] {
            badbit_ = true;
        } else {
            bytes_written_ = new_written_count;
        }
    }

    // Required by CRTP base (BasicOStream)
    void do_set_badbit()
    {
        badbit_ = true;
    }

    // size the buffer slightly bigger than requested as it allows
    // for higher performance outputting in OStreamBase. However, this
    // is an internal detail, not visible to user via API.
    static constexpr std::size_t BufSize
        = MaxN + OStreamBase<OStaticStream<MaxN>>::PutNumMaxBufRequest;

    char buf_[BufSize];
    std::size_t bytes_written_{0u};
    bool badbit_{false};
};

// Similar to std::ostream_iterator, but this works with any "Streamable" type.
template <class StreamT>
    requires Streamable<StreamT>
class OStreamIterator {
  public:
    OStreamIterator() = delete;

    explicit OStreamIterator(StreamT& os, const char* delim = nullptr) noexcept
    : os_(&os)
    , delim_(delim)
    {
    }

    template <class T>
    OStreamIterator& operator=(const T& value)
    {
        *os_ << value;
        if (delim_) [[unlikely]] {
            *os_ << delim_;
        }
        return *this;
    }

    OStreamIterator& operator*() noexcept { return *this; }
    OStreamIterator& operator++() noexcept { return *this; }
    OStreamIterator& operator++(int) noexcept { return *this; }

    // required by std::output_iterator concept
    using difference_type = ptrdiff_t;

  private:
    // Pointer (instead of reference) because this class needs to be assignable
    // to satisfy std::output_iterator concept (references can't be assigned).
    StreamT* os_;
    const char* delim_{nullptr};
};

// similar to std::experimental::ostream_joiner, but this works with any "Streamable" type.
template <class StreamT, class DelimT>
    requires Streamable<StreamT>
class OStreamJoiner {
  public:
    OStreamJoiner() = delete;
    ~OStreamJoiner() = default;

    explicit OStreamJoiner(StreamT& os) noexcept
    : os_(os)
    {
    }

    explicit OStreamJoiner(StreamT& os, DelimT delim)
        noexcept(std::is_nothrow_move_constructible_v<DelimT>)
    : os_(&os)
    , delim_(std::move(delim))
    {
    }

    template <class T>
    OStreamJoiner& operator=(const T& value)
    {
        if (!first_) {
            *os_ << delim_;
        }
        *os_ << value;
        first_ = false;
        return *this;
    }

    OStreamJoiner& operator*() noexcept { return *this; }
    OStreamJoiner& operator++() noexcept { return *this; }
    OStreamJoiner& operator++(int) noexcept { return *this; }

    // required by std::output_iterator concept
    using difference_type = ptrdiff_t;

  private:
    // Pointer (instead of reference) because this class needs to be assignable
    // to satisfy std::output_iterator concept (references can't be assigned).
    StreamT* os_;
    DelimT delim_;
    bool first_{true};
};

static_assert(std::output_iterator<OStreamIterator<OStream<4096>>, const char&>, "");
static_assert(std::output_iterator<OStreamJoiner<OStream<4096>, int>, const char&>, "");

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STREAM_HPP
