// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#ifndef TOOLBOX_NET_RESOLVER2_HPP
#define TOOLBOX_NET_RESOLVER2_HPP

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IpAddr.hpp>
#include <toolbox/net/Socket.hpp>

namespace toolbox {
inline namespace net {

/// Convert strings like www.google.com to 3www6google3com, where numbers are bytes that follow
TOOLBOX_API char* to_dns(const std::string& host, char* buff);

class TOOLBOX_API DnsServers {
  public:
    using Servers = std::vector<std::string>;

    explicit DnsServers(std::istream& resolv_conf,
                        std::vector<std::string> additional_nameservers = {});

    const Servers& servers() const noexcept;
    const std::string& server() const;

  private:
    Servers servers_;
};

struct DnsRequest {
    enum Type {
        A = 1,     // Ipv4 address
        NS = 2,    // Nameserver
        CNAME = 5, // Canonical name
        SOA = 6,   // Start of authority zone
        PTR = 12,  // Domain name pointer
        MX = 15,   // Mail server
    };
};

IpAddrV4 TOOLBOX_API get_host_by_name(const std::string& nameserver, const std::string& hostname,
                                      DnsRequest::Type query_type);

int TOOLBOX_API send_dns_request(const std::string& nameserver, const std::string& hostname,
                                 DnsRequest::Type query_type);

IpAddrV4 TOOLBOX_API get_dns_request(int fd, UdpEndpoint& endpoint);

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_RESOLVER_HPP
