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

#ifndef TOOLBOX_UTIL_STRINGBUF_HPP
#define TOOLBOX_UTIL_STRINGBUF_HPP

#include <toolbox/util/Compare.hpp>

#include <cstring>
#include <string_view>

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
    StringBuf(std::string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
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
    int compare(const StringBuf<MaxR>& rhs) const noexcept
    {
        return compare(rhs.data(), rhs.size());
    }
    int compare(std::string_view rhs) const noexcept { return compare(rhs.data(), rhs.size()); }
    constexpr const char* data() const noexcept { return buf_; }
    constexpr bool empty() const noexcept { return len_ == 0; }
    constexpr std::size_t size() const noexcept { return len_; }
    constexpr void clear() noexcept { len_ = 0; }

    void assign(std::string_view rhs) noexcept { assign(rhs.data(), rhs.size()); }
    void append(std::string_view rhs) noexcept { append(rhs.data(), rhs.size()); }
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

  private:
    void assign(const char* rdata, std::size_t rlen) noexcept
    {
        len_ = std::min(rlen, MaxN);
        if (len_ > 0) {
            std::memcpy(buf_, rdata, len_);
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
    int compare(const char* rdata, std::size_t rlen) const noexcept
    {
        int result{std::memcmp(buf_, rdata, std::min(size(), rlen))};
        if (result == 0) {
            result = toolbox::compare(size(), rlen);
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

template <std::size_t MaxL, std::size_t MaxR>
bool operator==(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
bool operator==(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) == 0;
}

template <std::size_t MaxN>
bool operator==(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 == rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
bool operator!=(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
bool operator!=(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) != 0;
}

template <std::size_t MaxN>
bool operator!=(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 != rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
bool operator<(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
bool operator<(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) < 0;
}

template <std::size_t MaxN>
bool operator<(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 < rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
bool operator<=(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
bool operator<=(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) <= 0;
}

template <std::size_t MaxN>
bool operator<=(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 <= rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
bool operator>(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
bool operator>(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) > 0;
}

template <std::size_t MaxN>
bool operator>(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 > rhs.compare(lhs);
}

template <std::size_t MaxL, std::size_t MaxR>
bool operator>=(const StringBuf<MaxL>& lhs, const StringBuf<MaxR>& rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
bool operator>=(const StringBuf<MaxN>& lhs, std::string_view rhs) noexcept
{
    return lhs.compare(rhs) >= 0;
}

template <std::size_t MaxN>
bool operator>=(std::string_view lhs, const StringBuf<MaxN>& rhs) noexcept
{
    return 0 >= rhs.compare(lhs);
}

template <std::size_t MaxN>
std::ostream& operator<<(std::ostream& os, const StringBuf<MaxN>& rhs)
{
    return std::operator<<(os, std::string_view{rhs.data(), rhs.size()});
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STRINGBUF_HPP
