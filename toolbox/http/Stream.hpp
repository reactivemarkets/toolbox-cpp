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

#ifndef TOOLBOX_HTTP_STREAM_HPP
#define TOOLBOX_HTTP_STREAM_HPP

#include <toolbox/http/Types.hpp>
#include <toolbox/io/Buffer.hpp>
#include <toolbox/util/Stream.hpp>

namespace toolbox {
inline namespace http {

constexpr char ApplicationJson[]{"application/json"};
constexpr char TextHtml[]{"text/html"};
constexpr char TextPlain[]{"text/plain"};

class TOOLBOX_API HttpBuf : public std::streambuf {
  public:
    explicit HttpBuf(Buffer& buf) noexcept
    : buf_{buf}
    {
    }
    ~HttpBuf() final;

    // Copy.
    HttpBuf(const HttpBuf&) = delete;
    HttpBuf& operator=(const HttpBuf&) = delete;

    // Move.
    HttpBuf(HttpBuf&&) = delete;
    HttpBuf& operator=(HttpBuf&&) = delete;

    std::streamsize pcount() const noexcept { return pcount_; }
    void commit() noexcept { buf_.commit(pcount_); }
    void reset() noexcept
    {
        pbase_ = nullptr;
        pcount_ = 0;
    }
    void set_content_length(std::streamsize pos, std::streamsize len) noexcept;

  protected:
    int_type overflow(int_type c) noexcept final;
    std::streamsize xsputn(const char_type* s, std::streamsize count) noexcept final;

  private:
    Buffer& buf_;
    char* pbase_{nullptr};
    std::streamsize pcount_{0};
};

class TOOLBOX_API HttpStream : public std::ostream {
  public:
    explicit HttpStream(Buffer& buf) noexcept
    : std::ostream{nullptr}
    , buf_{buf}
    {
        rdbuf(&buf_);
    }
    ~HttpStream() final;

    // Copy.
    HttpStream(const HttpStream&) = delete;
    HttpStream& operator=(const HttpStream&) = delete;

    // Move.
    HttpStream(HttpStream&&) = delete;
    HttpStream& operator=(HttpStream&&) = delete;

    std::streamsize pcount() const noexcept { return buf_.pcount(); }
    void commit() noexcept;
    void reset() noexcept
    {
        buf_.reset();
        toolbox::reset(*this);
        cloff_ = hcount_ = 0;
    }
    void reset(HttpStatus status, const char* content_type, NoCache no_cache = NoCache::Yes);

  private:
    HttpBuf buf_;
    /// Content-Length offset.
    std::streamsize cloff_{0};
    /// Header size.
    std::streamsize hcount_{0};
};

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_STREAM_HPP
