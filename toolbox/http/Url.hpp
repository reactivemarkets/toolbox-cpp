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

#ifndef TOOLBOX_HTTP_URL_HPP
#define TOOLBOX_HTTP_URL_HPP

#include <toolbox/http/Exception.hpp>

#include <toolbox/contrib/http_parser.h>

namespace toolbox {
inline namespace http {

template <typename DerivedT>
class BasicUrl {
  public:
    BasicUrl() noexcept { http_parser_url_init(&parser_); }

    // Copy.
    BasicUrl(const BasicUrl&) noexcept = default;
    BasicUrl& operator=(const BasicUrl&) noexcept = default;

    // Move.
    BasicUrl(BasicUrl&&) noexcept = default;
    BasicUrl& operator=(BasicUrl&&) noexcept = default;

    auto schema() const noexcept
    {
        const auto& field = parser_.field_data[UF_SCHEMA];
        return url().substr(field.off, field.len);
    }
    auto host() const noexcept
    {
        const auto& field = parser_.field_data[UF_HOST];
        return url().substr(field.off, field.len);
    }
    auto port() const noexcept
    {
        const auto& field = parser_.field_data[UF_PORT];
        return url().substr(field.off, field.len);
    }
    auto path() const noexcept
    {
        const auto& field = parser_.field_data[UF_PATH];
        return url().substr(field.off, field.len);
    }
    auto query() const noexcept
    {
        const auto& field = parser_.field_data[UF_QUERY];
        return url().substr(field.off, field.len);
    }
    auto fragment() const noexcept
    {
        const auto& field = parser_.field_data[UF_FRAGMENT];
        return url().substr(field.off, field.len);
    }
    auto user_info() const noexcept
    {
        const auto& field = parser_.field_data[UF_USERINFO];
        return url().substr(field.off, field.len);
    }

  protected:
    ~BasicUrl() = default;

    void reset() noexcept { http_parser_url_init(&parser_); }
    void parse(bool is_connect = false)
    {
        const auto rc
            = http_parser_parse_url(url().data(), url().size(), is_connect ? 1 : 0, &parser_);
        if (rc != 0) {
            throw HttpException{HttpStatus::BadRequest, err_msg() << "invalid url: " << url()};
        }
    }

  private:
    decltype(auto) url() const noexcept { return static_cast<const DerivedT*>(this)->url(); }
    http_parser_url parser_{};
};

class Url : public BasicUrl<Url> {
  public:
    explicit Url(const std::string& url)
    : url_{url}
    {
        parse();
    }

    // Copy.
    Url(const Url&) = default;
    Url& operator=(const Url&) = default;

    // Move.
    Url(Url&&) = default;
    Url& operator=(Url&&) = default;

    const auto& url() const noexcept { return url_; }

  private:
    std::string url_;
};

class UrlView : public BasicUrl<UrlView> {
  public:
    explicit UrlView(std::string_view url)
    : url_{url}
    {
        parse();
    }

    // Copy.
    UrlView(const UrlView&) noexcept = default;
    UrlView& operator=(const UrlView&) noexcept = default;

    // Move.
    UrlView(UrlView&&) noexcept = default;
    UrlView& operator=(UrlView&&) noexcept = default;

    const auto& url() const noexcept { return url_; }

  private:
    std::string_view url_;
};

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_URL_HPP
