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

#ifndef TOOLBOX_UTIL_STREAM_HPP
#define TOOLBOX_UTIL_STREAM_HPP

#include <toolbox/Config.h>

#include <experimental/iterator>

#include <ostream>
#include <string_view>

namespace toolbox {
inline namespace util {

TOOLBOX_API void reset(std::ostream& os) noexcept;

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
    operator std::string_view() const noexcept { return buf_.str(); }
    void reset() noexcept
    {
        buf_.reset();
        toolbox::reset(*this);
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
