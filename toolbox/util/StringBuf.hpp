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

#ifndef TOOLBOX_UTIL_STRINGBUF_HPP
#define TOOLBOX_UTIL_STRINGBUF_HPP

#include <toolbox/util/Concepts.hpp>

#include <cstring>
#include <string_view>

#include <boost/container_hash/hash.hpp>

namespace toolbox {
inline namespace util {

/// String buffer with fixed upper-bound.
template <std::size_t MaxN>
class StringBuf {
  public:
    template <std::size_t MaxR>
    StringBuf(const StringBuf<MaxR>& rhs) noexcept
    {
        assign(rhs.data(), rhs.size());
    }
    explicit StringBuf(std::string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
    constexpr StringBuf() noexcept = default;

    ~StringBuf() = default;

    // Copy.
    StringBuf(const StringBuf& rhs) noexcept { assign(rhs.data(), rhs.size()); }
    StringBuf& operator=(const StringBuf& rhs) noexcept
    {
        assign(rhs.data(), rhs.size());
        return *this;
    }

    // Move.
    constexpr StringBuf(StringBuf&&) noexcept = default;
    constexpr StringBuf& operator=(StringBuf&&) noexcept = default;

    template <std::size_t MaxR>
    StringBuf& operator=(const StringBuf<MaxR>& rhs) noexcept
    {
        assign(rhs.data(), rhs.size());
        return *this;
    }
    StringBuf& operator=(std::string_view rhs) noexcept
    {
        assign(rhs.data(), rhs.size());
        return *this;
    }
    template <std::size_t MaxR>
    StringBuf& operator+=(const StringBuf<MaxR>& rhs) noexcept
    {
        append(rhs.data(), rhs.size());
        return *this;
    }
    StringBuf& operator+=(std::string_view rhs) noexcept
    {
        append(rhs.data(), rhs.size());
        return *this;
    }

    constexpr const char* data() const noexcept { return buf_; }
    constexpr bool empty() const noexcept { return len_ == 0; }
    constexpr std::size_t size() const noexcept { return len_; }
    constexpr void clear() noexcept { len_ = 0; }

    void assign(std::string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
    void append(std::string_view rhs) noexcept { append(rhs.data(), rhs.size()); }

    template <typename TypeT>
    auto operator<=>(const TypeT& rhs) const noexcept
    {
        return compare(rhs.data(), rhs.size());
    }
    template <typename TypeT>
    bool operator==(const TypeT& rhs) const noexcept
    {
        return size() == rhs.size() && compare(rhs.data(), rhs.size()) == 0;
    }

    explicit constexpr operator std::string_view() const noexcept
    {
        return std::string_view{data(), size()};
    }

  private:
    void assign(const char* rdata, std::size_t rlen) noexcept
    {
        len_ = std::min(rlen, MaxN);
        if (len_ > 0) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnonnull"
            std::memcpy(buf_, rdata, len_);
#pragma GCC diagnostic pop
        }
    }
    void append(const char* rdata, std::size_t rlen) noexcept
    {
        rlen = std::min(rlen, MaxN - len_);
        if (rlen > 0) {
            std::memcpy(buf_ + len_, rdata, rlen);
            len_ += rlen;
        }
    }
    auto compare(const char* rdata, std::size_t rlen) const noexcept
    {
        std::strong_ordering result{std::memcmp(buf_, rdata, std::min(size(), rlen)) <=> 0};
        if (result == nullptr) {
            result = size() <=> rlen;
        }
        return result;
    }
    // Length in the first cache-line.
    // Use int to save space.
    int len_{0};
    char buf_[MaxN];
};

template <std::size_t MaxN>
constexpr std::string_view operator+(const StringBuf<MaxN>& s) noexcept
{
    return {s.data(), s.size()};
}

template <std::size_t MaxN, typename StreamT>
    requires Streamable<StreamT>
StreamT& operator<<(StreamT& os, const StringBuf<MaxN>& rhs)
{
    os << std::string_view{rhs.data(), rhs.size()};
    return os;
}

} // namespace util
} // namespace toolbox

namespace std {
template <std::size_t MaxN>
struct hash<toolbox::util::StringBuf<MaxN>> {
    inline std::size_t operator()(const toolbox::util::StringBuf<MaxN>& key) const
    {
        std::size_t h{0};
        boost::hash_combine(h, +key);
        return h;
    }
};
} // namespace std

#endif // TOOLBOX_UTIL_STRINGBUF_HPP
