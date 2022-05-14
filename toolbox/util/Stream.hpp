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

#include <toolbox/util/Storage.hpp>

#include <experimental/iterator>

namespace toolbox {
inline namespace util {
namespace detail {

struct ResetState {
};
TOOLBOX_API std::ostream& operator<<(std::ostream& os, ResetState) noexcept;

} // namespace detail

/// I/O manipulator that resets I/O state.
constexpr detail::ResetState reset_state{};

TOOLBOX_API void reset(std::ostream& os) noexcept;

/// StreamBuf uses a dynamic storage acquired from the custom allocator.
template <std::size_t MaxN>
class StreamBuf final : public std::streambuf {
  public:
    /// Constructor for initialising the StreamBuf with a null buffer.
    explicit StreamBuf(std::nullptr_t) noexcept {}
    StreamBuf()
    : storage_{make_storage<MaxN>}
    {
        setp(storage_->begin(), storage_->end());
    }
    ~StreamBuf() override = default;

    // Copy.
    StreamBuf(const StreamBuf&) = delete;
    StreamBuf& operator=(const StreamBuf&) = delete;

    // Move.
    StreamBuf(StreamBuf&&) = delete;
    StreamBuf& operator=(StreamBuf&&) = delete;

    const char* data() const noexcept { return pbase(); }
    bool empty() const noexcept { return pbase() == pptr(); }
    std::size_t size() const noexcept { return pptr() - pbase(); }

    /// Release the managed storage.
    StoragePtr<MaxN> release_storage() noexcept
    {
        StoragePtr<MaxN> storage;
        storage_.swap(storage);
        setp(nullptr, nullptr);
        return storage;
    }
    /// Update the internal storage.
    void set_storage(StoragePtr<MaxN> storage) noexcept { swap_storage(storage); }
    /// Swap the internal storage.
    void swap_storage(StoragePtr<MaxN>& storage) noexcept
    {
        storage_.swap(storage);
        if (storage_) {
            auto* const begin = static_cast<char*>(storage_.get());
            setp(begin, begin + MaxN);
        } else {
            setp(nullptr, nullptr);
        }
    }
    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept
    {
        if (storage_) {
            setp(storage_->begin(), storage_->end());
        }
    }

  private:
    StoragePtr<MaxN> storage_;
};

/// OStream uses a dynamic storage acquired from the custom allocator.
template <std::size_t MaxN>
class OStream final : public std::ostream {
  public:
    /// Constructor for initialising the StreamBuf with a null buffer.
    explicit OStream(std::nullptr_t) noexcept
    : std::ostream{nullptr}
    , buf_{nullptr}
    {
        rdbuf(&buf_);
    }
    OStream()
    : std::ostream{nullptr}
    {
        rdbuf(&buf_);
    }
    ~OStream() override = default;

    // Copy.
    OStream(const OStream&) = delete;
    OStream& operator=(const OStream&) = delete;

    // Move.
    OStream(OStream&&) = delete;
    OStream& operator=(OStream&&) = delete;

    static StoragePtr<MaxN> make_storage() { return util::make_storage<MaxN>(); }
    const char* data() const noexcept { return buf_.data(); }
    bool empty() const noexcept { return buf_.empty(); }
    std::size_t size() const noexcept { return buf_.size(); }

    /// Release the managed storage.
    StoragePtr<MaxN> release_storage() noexcept { return buf_.release_storage(); }
    /// Update the internal storage.
    void set_storage(StoragePtr<MaxN> storage) noexcept
    {
        return buf_.set_storage(std::move(storage));
    }
    /// Swap the internal storage.
    void swap_storage(StoragePtr<MaxN>& storage) noexcept { buf_.swap_storage(storage); }
    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept
    {
        buf_.reset();
        clear();
    }

  private:
    StreamBuf<MaxN> buf_;
};

template <std::size_t MaxN>
class StaticStreamBuf final : public std::streambuf {
  public:
    StaticStreamBuf() noexcept { reset(); }
    ~StaticStreamBuf() override = default;

    // Copy.
    StaticStreamBuf(const StaticStreamBuf&) = delete;
    StaticStreamBuf& operator=(const StaticStreamBuf&) = delete;

    // Move.
    StaticStreamBuf(StaticStreamBuf&&) = delete;
    StaticStreamBuf& operator=(StaticStreamBuf&&) = delete;

    const char* data() const noexcept { return pbase(); }
    bool empty() const noexcept { return pbase() == pptr(); }
    std::size_t size() const noexcept { return pptr() - pbase(); }

    std::string_view str() const noexcept { return {data(), size()}; }
    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept { setp(buf_, buf_ + MaxN); };

  private:
    char buf_[MaxN];
};

template <std::size_t MaxN>
class OStaticStream final : public std::ostream {
  public:
    OStaticStream()
    : std::ostream{nullptr}
    {
        rdbuf(&buf_);
    }
    ~OStaticStream() override = default;

    // Copy.
    OStaticStream(const OStaticStream&) = delete;
    OStaticStream& operator=(const OStaticStream&) = delete;

    // Move.
    OStaticStream(OStaticStream&&) = delete;
    OStaticStream& operator=(OStaticStream&&) = delete;

    const char* data() const noexcept { return buf_.data(); }
    bool empty() const noexcept { return buf_.empty(); }
    std::size_t size() const noexcept { return buf_.size(); }

    std::string_view str() const noexcept { return buf_.str(); }
    operator std::string_view() const noexcept
    {
        return buf_.str();
    } // NOLINT(hicpp-explicit-conversions)
    /// Reset the current position back to the beginning of the buffer.
    void reset() noexcept
    {
        buf_.reset();
        clear();
    };

  private:
    StaticStreamBuf<MaxN> buf_;
};

template <std::size_t MaxN, typename ValueT>
auto& operator<<(OStaticStream<MaxN>& os, ValueT&& val)
{
    static_cast<std::ostream&>(os) << std::forward<ValueT>(val);
    return os;
}

using OStreamJoiner = std::experimental::ostream_joiner<char>;

template <auto DelimT, typename ArgT, typename... ArgsT>
void join(std::ostream& os, const ArgT& arg, const ArgsT&... args)
{
    os << arg;
    (..., [&os](const auto& arg) { os << DelimT << arg; }(args));
}

} // namespace util
} // namespace toolbox

namespace std::experimental {
template <typename ValueT>
ostream_joiner<char>& operator<<(ostream_joiner<char>& osj, const ValueT& value)
{
    osj = value;
    return osj;
}
} // namespace std::experimental

#endif // TOOLBOX_UTIL_STREAM_HPP
