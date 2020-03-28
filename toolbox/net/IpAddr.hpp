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

#ifndef TOOLBOX_NET_IPADDR_HPP
#define TOOLBOX_NET_IPADDR_HPP

#include <boost/asio/ip/address.hpp>

namespace toolbox {
inline namespace net {
using IpAddr = boost::asio::ip::address;
using IpAddrV4 = boost::asio::ip::address_v4;
using IpAddrV6 = boost::asio::ip::address_v6;
} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_IPADDR_HPP
