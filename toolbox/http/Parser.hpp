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

#ifndef TOOLBOX_HTTP_PARSER_HPP
#define TOOLBOX_HTTP_PARSER_HPP

#include <toolbox/http/Exception.hpp>
#include <toolbox/http/Types.hpp>
#include <toolbox/io/Buffer.hpp>
#include <toolbox/sys/Time.hpp>

#include <string_view>

namespace toolbox {
inline namespace http {

template <typename DerivedT>
class BasicHttpParser {
  public:
    explicit BasicHttpParser(HttpType type) noexcept
    : type_{type}
    {
        // The http_parser_init() function preserves "data".
        // Important: cast is required for CRTP to work correctly with multiple inheritance.
        parser_.data = static_cast<DerivedT*>(this);
        http_parser_init(&parser_, static_cast<http_parser_type>(type));
        last_header_elem_ = None;
    }

    // Copy.
    BasicHttpParser(const BasicHttpParser&) noexcept = default;
    BasicHttpParser& operator=(const BasicHttpParser&) noexcept = default;

    // Move.
    BasicHttpParser(BasicHttpParser&&) noexcept = default;
    BasicHttpParser& operator=(BasicHttpParser&&) noexcept = default;

    int http_major() const noexcept { return parser_.http_major; }
    int http_minor() const noexcept { return parser_.http_minor; }
    int status_code() const noexcept { return parser_.status_code; }
    HttpMethod method() const noexcept { return static_cast<HttpMethod>(parser_.method); }
    bool should_keep_alive() const noexcept { return http_should_keep_alive(&parser_) != 0; }
    bool body_is_final() const noexcept { return http_body_is_final(&parser_) != 0; }

    void pause() noexcept { http_parser_pause(&parser_, 1); }

  protected:
    ~BasicHttpParser() = default;

    void reset() noexcept
    {
        // The http_parser_init() function preserves "data".
        http_parser_init(&parser_, static_cast<http_parser_type>(type_));
        last_header_elem_ = None;
    }
    std::size_t parse(CyclTime now, ConstBuffer buf)
    {
        static http_parser_settings settings{make_settings()};
        const auto rc = http_parser_execute(&parser_, &settings, buffer_cast<const char*>(buf),
                                            buffer_size(buf));
        const auto err = static_cast<http_errno>(parser_.http_errno);
        if (err != HPE_OK) {
            if (err == HPE_PAUSED) {
                // Clear pause state.
                http_parser_pause(&parser_, 0);
            } else {
                throw HttpException{HttpStatus::BadRequest,
                                    err_msg() << http_errno_name(err) << ": "
                                              << http_errno_description(err)};
            }
        }
        return rc;
    }

  private:
    static http_parser_settings make_settings() noexcept
    {
        // Important: callbacks must return 0 on success. Returning a non-zero value indicates error
        // to the parser, making it exit immediately.
        http_parser_settings settings{};
        settings.on_message_begin = on_message_begin;
        settings.on_url = on_url;
        settings.on_status = on_status;
        settings.on_header_field = on_header_field;
        settings.on_header_value = on_header_value;
        settings.on_headers_complete = on_headers_end;
        settings.on_body = on_body;
        settings.on_message_complete = on_message_end;
        settings.on_chunk_header = on_chunk_header;
        settings.on_chunk_complete = on_chunk_end;
        return settings;
    }
    static int on_message_begin(http_parser* parser) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_message_begin(CyclTime::current()) ? 0 : -1;
    }
    static int on_url(http_parser* parser, const char* at, std::size_t length) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_url(CyclTime::current(), {at, length}) ? 0
                                                                                               : -1;
    }
    static int on_status(http_parser* parser, const char* at, std::size_t length) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_status(CyclTime::current(), {at, length})
            ? 0
            : -1;
    }
    static int on_header_field(http_parser* parser, const char* at, std::size_t length) noexcept
    {
        auto* const obj = static_cast<DerivedT*>(parser->data);
        First first;
        if (obj->last_header_elem_ != Field) {
            obj->last_header_elem_ = Field;
            first = First::Yes;
        } else {
            first = First::No;
        }
        return obj->on_header_field(CyclTime::current(), {at, length}, first) ? 0 : -1;
    }
    static int on_header_value(http_parser* parser, const char* at, std::size_t length) noexcept
    {
        auto* const obj = static_cast<DerivedT*>(parser->data);
        First first;
        if (obj->last_header_elem_ != Value) {
            obj->last_header_elem_ = Value;
            first = First::Yes;
        } else {
            first = First::No;
        }
        return obj->on_header_value(CyclTime::current(), {at, length}, first) ? 0 : -1;
    }
    static int on_headers_end(http_parser* parser) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_headers_end(CyclTime::current()) ? 0 : -1;
    }
    static int on_body(http_parser* parser, const char* at, std::size_t length) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_body(CyclTime::current(), {at, length})
            ? 0
            : -1;
    }
    static int on_message_end(http_parser* parser) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_message_end(CyclTime::current()) ? 0 : -1;
    }
    static int on_chunk_header(http_parser* parser) noexcept
    {
        // When on_chunk_header is called, the current chunk length is stored in parser->content_length.
        return static_cast<DerivedT*>(parser->data)
                   ->on_chunk_header(CyclTime::current(), parser->content_length)
            ? 0
            : -1;
    }
    static int on_chunk_end(http_parser* parser) noexcept
    {
        return static_cast<DerivedT*>(parser->data)->on_chunk_end(CyclTime::current()) ? 0 : -1;
    }
    HttpType type_;
    http_parser parser_;
    enum { None = 0, Field, Value } last_header_elem_;
};

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_PARSER_HPP
