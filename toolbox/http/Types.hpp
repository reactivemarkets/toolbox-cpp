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

#ifndef TOOLBOX_HTTP_TYPES_HPP
#define TOOLBOX_HTTP_TYPES_HPP

#include <toolbox/contrib/http_parser.h>

#include <iostream>

namespace toolbox {
inline namespace http {

enum class First : bool { No = false, Yes = true };

enum class NoCache : bool { No = false, Yes = true };

enum class HttpMethod : int {
    Delete = HTTP_DELETE,
    Get = HTTP_GET,
    Head = HTTP_HEAD,
    Post = HTTP_POST,
    Put = HTTP_PUT,
    Connect = HTTP_CONNECT,
    Options = HTTP_OPTIONS,
    Trace = HTTP_TRACE,
    Copy = HTTP_COPY,
    Lock = HTTP_LOCK,
    MkCol = HTTP_MKCOL,
    Move = HTTP_MOVE,
    PropFind = HTTP_PROPFIND,
    PropPatch = HTTP_PROPPATCH,
    Search = HTTP_SEARCH,
    Unlock = HTTP_UNLOCK,
    Bind = HTTP_BIND,
    Rebind = HTTP_REBIND,
    Unbind = HTTP_UNBIND,
    Acl = HTTP_ACL,
    Report = HTTP_REPORT,
    MkActivity = HTTP_MKACTIVITY,
    Checkout = HTTP_CHECKOUT,
    Merge = HTTP_MERGE,
    MSearch = HTTP_MSEARCH,
    Notify = HTTP_NOTIFY,
    Subscribe = HTTP_SUBSCRIBE,
    Unsubscribe = HTTP_UNSUBSCRIBE,
    Patch = HTTP_PATCH,
    Purge = HTTP_PURGE,
    MkCalendar = HTTP_MKCALENDAR,
    Link = HTTP_LINK,
    Unlink = HTTP_UNLINK,
    Source = HTTP_SOURCE
};

inline const char* enum_string(HttpMethod method) noexcept
{
    return http_method_str(static_cast<http_method>(method));
}

inline std::ostream& operator<<(std::ostream& os, HttpMethod method)
{
    return os << enum_string(method);
}

enum class HttpStatus : int {
    Ok = HTTP_STATUS_OK,
    NoContent = HTTP_STATUS_NO_CONTENT,
    BadRequest = HTTP_STATUS_BAD_REQUEST,
    Unauthorized = HTTP_STATUS_UNAUTHORIZED,
    Forbidden = HTTP_STATUS_FORBIDDEN,
    NotFound = HTTP_STATUS_NOT_FOUND,
    MethodNotAllowed = HTTP_STATUS_METHOD_NOT_ALLOWED,
    InternalServerError = HTTP_STATUS_INTERNAL_SERVER_ERROR,
    ServiceUnavailable = HTTP_STATUS_SERVICE_UNAVAILABLE
};

TOOLBOX_API const char* enum_string(HttpStatus status) noexcept;

inline std::ostream& operator<<(std::ostream& os, HttpStatus status)
{
    return os << static_cast<int>(status);
}

enum class HttpType : int { Request = HTTP_REQUEST, Response = HTTP_RESPONSE };

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_TYPES_HPP
