// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#ifndef TOOLBOX_HTTP_SERV_HPP
#define TOOLBOX_HTTP_SERV_HPP

#include <toolbox/http/Conn.hpp>
#include <toolbox/net/StreamAcceptor.hpp>

namespace toolbox {
inline namespace http {

template <typename ConnT, typename AppT>
class BasicServ : public StreamAcceptor<BasicServ<ConnT, AppT>> {

    friend StreamAcceptor<BasicServ<ConnT, AppT>>;

    using Conn = ConnT;
    using App = AppT;
    using ConstantTimeSizeOption = boost::intrusive::constant_time_size<false>;
    using MemberHookOption
        = boost::intrusive::member_hook<Conn, decltype(Conn::list_hook), &Conn::list_hook>;
    using ConnList = boost::intrusive::list<Conn, ConstantTimeSizeOption, MemberHookOption>;

    using typename StreamAcceptor<BasicServ<ConnT, AppT>>::Endpoint;

  public:
    BasicServ(CyclTime /*now*/, Reactor& r, const Endpoint& ep, App& app)
    : StreamAcceptor<BasicServ<ConnT, AppT>>{r, ep}
    , reactor_{r}
    , app_{app}
    {
    }
    ~BasicServ()
    {
        const auto now = CyclTime::current();
        conn_list_.clear_and_dispose([now](auto* conn) { conn->dispose(now); });
    }

    // Copy.
    BasicServ(const BasicServ&) = delete;
    BasicServ& operator=(const BasicServ&) = delete;

    // Move.
    BasicServ(BasicServ&&) = delete;
    BasicServ& operator=(BasicServ&&) = delete;

  private:
    void on_sock_prepare(CyclTime /*now*/, IoSock& /*sock*/) {}
    void on_sock_accept(CyclTime now, IoSock&& sock, const Endpoint& ep)
    {
        auto* const conn = new Conn{now, reactor_, std::move(sock), ep, app_};
        conn_list_.push_back(*conn);
    }

    Reactor& reactor_;
    App& app_;
    // List of active connections.
    ConnList conn_list_;
};

using Serv = BasicServ<Conn, App>;

} // namespace http
} // namespace toolbox

#endif // TOOLBOX_HTTP_SERV_HPP
