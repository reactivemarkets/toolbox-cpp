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

#include "Parser.hpp"

#include <toolbox/http/Url.hpp>
#include <toolbox/util/String.hpp>

#include <boost/test/unit_test.hpp>

#include <iostream>
#include <vector>

namespace std {
template <typename T, typename U>
ostream& operator<<(ostream& os, const pair<T, U>& p)
{
    return os << '(' << p.first << ',' << p.second << ')';
}
} // namespace std

using namespace std;
using namespace toolbox;

namespace {

class HttpParser
: public BasicHttpParser<HttpParser>
, public BasicUrl<HttpParser> {
    friend class BasicHttpParser<HttpParser>;
    friend class BasicUrl<HttpParser>;

  public:
    using BasicHttpParser<HttpParser>::BasicHttpParser;
    ~HttpParser() = default;

    const auto& url() const noexcept { return url_; }
    const auto& status() const noexcept { return status_; }
    const auto& headers() const noexcept { return headers_; }
    const auto& body() const noexcept { return body_; }

    void clear() noexcept
    {
        url_.clear();
        status_.clear();
        headers_.clear();
        body_.clear();
    }
    using BasicHttpParser<HttpParser>::parse;

  private:
    bool on_message_begin(CyclTime now) noexcept
    {
        BasicUrl<HttpParser>::reset();
        clear();
        return true;
    }
    bool on_url(CyclTime now, string_view sv) noexcept
    {
        url_.append(sv.data(), sv.size());
        return true;
    }
    bool on_status(CyclTime now, string_view sv) noexcept
    {
        status_.append(sv.data(), sv.size());
        return true;
    }
    bool on_header_field(CyclTime now, string_view sv, First first) noexcept
    {
        if (first == First::Yes) {
            headers_.emplace_back(string{sv.data(), sv.size()}, "");
        } else {
            headers_.back().first.append(sv.data(), sv.size());
        }
        return true;
    }
    bool on_header_value(CyclTime now, string_view sv, First first) noexcept
    {
        headers_.back().second.append(sv.data(), sv.size());
        return true;
    }
    bool on_headers_end(CyclTime now) noexcept { return true; }
    bool on_body(CyclTime now, string_view sv) noexcept
    {
        body_.append(sv.data(), sv.size());
        return true;
    }
    bool on_message_end(CyclTime now) noexcept
    {
        bool ret{false};
        try {
            if (!url_.empty()) {
                BasicUrl<HttpParser>::parse();
            }
            ret = true;
        } catch (const std::exception& e) {
            cerr << "exception: " << e.what() << endl;
        }
        return ret;
    }
    bool on_chunk_header(CyclTime now, size_t len) noexcept { return true; }
    bool on_chunk_end(CyclTime now) noexcept { return true; }

    string url_;
    string status_;
    vector<pair<string, string>> headers_;
    string body_;
};

} // namespace

BOOST_AUTO_TEST_SUITE(ParserSuite)

BOOST_AUTO_TEST_CASE(HttpInitialRequestLineCase)
{
    constexpr auto Message =                        //
        "GET /path/to/file/index.html HTTP/1.0\r\n" //
        "\r\n"sv;

    HttpParser h{HttpType::Request};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(!h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 0);

    BOOST_TEST(h.method() == HttpMethod::Get);
    BOOST_TEST(h.url() == "/path/to/file/index.html"s);

    BOOST_TEST(h.headers().empty());
    BOOST_TEST(h.body().empty());
}

BOOST_AUTO_TEST_CASE(HttpInitialResponseLineCase)
{
    constexpr auto Message =         //
        "HTTP/1.0 404 Not Found\r\n" //
        "\r\n"sv;

    HttpParser h{HttpType::Response};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(!h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 0);

    BOOST_TEST(h.status_code() == 404);
    BOOST_TEST(h.status() == "Not Found"s);

    BOOST_TEST(h.headers().empty());
    BOOST_TEST(h.body().empty());
}

BOOST_AUTO_TEST_CASE(HttpBasicRequestCase)
{
    constexpr auto Message =                     //
        "GET /path/file.html HTTP/1.0\r\n"       //
        "From: someuser@reactivemarkets.com\r\n" //
        "User-Agent: HTTPTool/1.0\r\n"           //
        "\r\n"sv;

    HttpParser h{HttpType::Request};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(!h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 0);

    BOOST_TEST(h.method() == HttpMethod::Get);
    BOOST_TEST(h.url() == "/path/file.html"s);

    BOOST_TEST(h.headers().size() == 2U);
    BOOST_TEST(h.headers()[0] == make_pair("From"s, "someuser@reactivemarkets.com"s));
    BOOST_TEST(h.headers()[1] == make_pair("User-Agent"s, "HTTPTool/1.0"s));

    BOOST_TEST(h.body().empty());
}

BOOST_AUTO_TEST_CASE(HttpBasicResponseCase)
{
    constexpr auto Message =                      //
        "HTTP/1.0 200 OK\r\n"                     //
        "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n" //
        "Content-Type: text/plain\r\n"            //
        "Content-Length: 13\r\n"                  //
        "\r\n"                                    //
        "Hello, World!"sv;

    HttpParser h{HttpType::Response};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(!h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 0);

    BOOST_TEST(h.status_code() == 200);
    BOOST_TEST(h.status() == "OK"s);

    BOOST_TEST(h.headers().size() == 3U);
    BOOST_TEST(h.headers()[0] == make_pair("Date"s, "Fri, 31 Dec 1999 23:59:59 GMT"s));
    BOOST_TEST(h.headers()[1] == make_pair("Content-Type"s, "text/plain"s));
    BOOST_TEST(h.headers()[2] == make_pair("Content-Length"s, "13"s));

    BOOST_TEST(h.body() == "Hello, World!"s);
}

BOOST_AUTO_TEST_CASE(HttpPostRequestCase)
{
    constexpr auto Message =                                  //
        "POST /path/script.cgi HTTP/1.0\r\n"                  //
        "From: frog@reactivemarkets.com\r\n"                  //
        "User-Agent: HTTPTool/1.0\r\n"                        //
        "Content-Type: application/x-www-form-urlencoded\r\n" //
        "Content-Length: 32\r\n"                              //
        "\r\n"                                                //
        "home=Cosby&favorite+flavor=flies"sv;

    HttpParser h{HttpType::Request};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(!h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 0);

    BOOST_TEST(h.method() == HttpMethod::Post);
    BOOST_TEST(h.url() == "/path/script.cgi"s);

    BOOST_TEST(h.headers().size() == 4U);
    BOOST_TEST(h.headers()[0] == make_pair("From"s, "frog@reactivemarkets.com"s));
    BOOST_TEST(h.headers()[1] == make_pair("User-Agent"s, "HTTPTool/1.0"s));
    BOOST_TEST(h.headers()[2] == make_pair("Content-Type"s, "application/x-www-form-urlencoded"s));
    BOOST_TEST(h.headers()[3] == make_pair("Content-Length"s, "32"s));

    BOOST_TEST(h.body() == "home=Cosby&favorite+flavor=flies"s);
}

BOOST_AUTO_TEST_CASE(HttpKeepAliveRequestCase)
{
    constexpr auto Message =               //
        "GET /path/file.html HTTP/1.1\r\n" //
        "Host: www.host1.com:80\r\n"       //
        "\r\n"sv;

    HttpParser h{HttpType::Request};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 1);

    BOOST_TEST(h.method() == HttpMethod::Get);
    BOOST_TEST(h.url() == "/path/file.html"s);

    BOOST_TEST(h.headers().size() == 1U);
    BOOST_TEST(h.headers()[0] == make_pair("Host"s, "www.host1.com:80"s));

    BOOST_TEST(h.body().empty());
}

BOOST_AUTO_TEST_CASE(HttpChunkedResponseCase)
{
    constexpr auto Message =                      //
        "HTTP/1.1 200 OK\r\n"                     //
        "Date: Fri, 31 Dec 1999 23:59:59 GMT\r\n" //
        "Content-Type: text/plain\r\n"            //
        "Transfer-Encoding: chunked\r\n"          //
        "\r\n"                                    //
        "1a; ignore-stuff-here\r\n"               //
        "abcdefghijklmnopqrstuvwxyz\r\n"          //
        "10\r\n"                                  //
        "1234567890abcdef\r\n"                    //
        "0\r\n"                                   //
        "some-footer: some-value\r\n"             //
        "another-footer: another-value\r\n"       //
        "\r\n"sv;

    HttpParser h{HttpType::Response};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 1);

    BOOST_TEST(h.status_code() == 200);
    BOOST_TEST(h.status() == "OK"s);

    BOOST_TEST(h.headers().size() == 5U);
    BOOST_TEST(h.headers()[0] == make_pair("Date"s, "Fri, 31 Dec 1999 23:59:59 GMT"s));
    BOOST_TEST(h.headers()[1] == make_pair("Content-Type"s, "text/plain"s));
    BOOST_TEST(h.headers()[2] == make_pair("Transfer-Encoding"s, "chunked"s));
    BOOST_TEST(h.headers()[3] == make_pair("some-footer"s, "some-value"s));
    BOOST_TEST(h.headers()[4] == make_pair("another-footer"s, "another-value"s));

    BOOST_TEST(h.body() == "abcdefghijklmnopqrstuvwxyz1234567890abcdef"s);
}

BOOST_AUTO_TEST_CASE(HttpMultiResponseCase)
{
    constexpr auto Message =                 //
        "POST /path/script.cgi HTTP/1.1\r\n" //
        "Content-Type: text/plain\r\n"       //
        "Content-Length: 5\r\n"              //
        "\r\n"                               //
        "first"                              //
        "POST /path/script.cgi HTTP/1.1\r\n" //
        "Content-Type: text/plain\r\n"       //
        "Content-Length: 6\r\n"              //
        "\r\n"                               //
        "second"sv;

    HttpParser h{HttpType::Request};
    const auto now = CyclTime::now();
    BOOST_TEST(h.parse(now, {Message.data(), Message.size()}) == Message.size());
    BOOST_TEST(h.should_keep_alive());
    BOOST_TEST(h.http_major() == 1);
    BOOST_TEST(h.http_minor() == 1);

    BOOST_TEST(h.method() == HttpMethod::Post);
    BOOST_TEST(h.url() == "/path/script.cgi"s);

    BOOST_TEST(h.headers().size() == 2U);
    BOOST_TEST(h.headers()[0] == make_pair("Content-Type"s, "text/plain"s));
    BOOST_TEST(h.headers()[1] == make_pair("Content-Length"s, "6"s));

    BOOST_TEST(h.body() == "second"s);
}

BOOST_AUTO_TEST_SUITE_END()
