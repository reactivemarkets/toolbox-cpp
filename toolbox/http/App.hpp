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

#ifndef TOOLBOX_HTTP_APP_HPP
#define TOOLBOX_HTTP_APP_HPP

#include <toolbox/http/Conn.hpp>
#include <toolbox/sys/Time.hpp>
#include <toolbox/util/Version.hpp>

namespace toolbox {
inline namespace http {

class HttpRequest;
class HttpStream;

class TOOLBOX_API HttpAppBase {
  public:
    using Protocol = StreamProtocol;
    using Endpoint = StreamEndpoint;

    HttpAppBase() noexcept = default;
    virtual ~HttpAppBase();

    // Copy.
    constexpr HttpAppBase(const HttpAppBase&) noexcept = default;
    HttpAppBase& operator=(const HttpAppBase&) noexcept = default;

    // Move.
    constexpr HttpAppBase(HttpAppBase&&) noexcept = default;
    HttpAppBase& operator=(HttpAppBase&&) noexcept = default;

    void on_http_connect(CyclTime now, const Endpoint& ep) { do_on_http_connect(now, ep); }
    void on_http_disconnect(CyclTime now, const Endpoint& ep) noexcept
    {
        do_on_http_disconnect(now, ep);
    }
    void on_http_error(CyclTime now, const Endpoint& ep, const std::exception& e,
                       HttpStream& os) noexcept
    {
        do_on_http_error(now, ep, e, os);
    }
    void on_http_message(CyclTime now, const Endpoint& ep, const HttpRequest& req, HttpStream& os)
    {
        do_on_http_message(now, ep, req, os);
    }
    void on_http_timeout(CyclTime now, const Endpoint& ep) noexcept { do_on_http_timeout(now, ep); }

  protected:
    virtual void do_on_http_connect(CyclTime now, const Endpoint& ep) = 0;
    virtual void do_on_http_disconnect(CyclTime now, const Endpoint& ep) noexcept = 0;
    virtual void do_on_http_error(CyclTime now, const Endpoint& ep, const std::exception& e,
                                  HttpStream& os) noexcept
        = 0;
    virtual void do_on_http_message(CyclTime now, const Endpoint& ep, const HttpRequest& req,
                                    HttpStream& os)
        = 0;
    virtual void do_on_http_timeout(CyclTime now, const Endpoint& ep) noexcept = 0;
};

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_APP_HPP
