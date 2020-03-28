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

#include "Reactor.hpp"

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/net/IoSock.hpp>
#include <toolbox/util/RefCount.hpp>

#include <boost/test/unit_test.hpp>

using namespace std;
using namespace toolbox;

namespace {

struct TestHandler : RefCount<TestHandler, ThreadUnsafePolicy> {
    void on_input(CyclTime now, int fd, unsigned events)
    {
        char buf[4];
        os::recv(fd, buf, 4, 0);
        if (strcmp(buf, "foo") == 0) {
            ++matches;
        }
    }
    int matches{};
};

} // namespace

BOOST_AUTO_TEST_SUITE(ReactorSuite)

BOOST_AUTO_TEST_CASE(ReactorLevelCase)
{
    using namespace literals::chrono_literals;

    Reactor r{1024};
    auto h = make_intrusive<TestHandler>();

    auto socks = socketpair(UnixStreamProtocol{});
    const auto sub = r.subscribe(*socks.second, EpollIn, bind<&TestHandler::on_input>(h.get()));

    const auto now = CyclTime::now();
    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 0);

    socks.first.send("foo", 4, 0);
    socks.first.send("foo", 4, 0);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 1);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 2);

    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 2);

    socks.first.send("foo", 4, 0);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 3);

    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 3);
}

BOOST_AUTO_TEST_CASE(ReactorEdgeCase)
{
    using namespace literals::chrono_literals;

    Reactor r{1024};
    auto h = make_intrusive<TestHandler>();

    auto socks = socketpair(UnixStreamProtocol{});
    auto sub = r.subscribe(*socks.second, EpollIn | EpollEt, bind<&TestHandler::on_input>(h.get()));

    const auto now = CyclTime::now();
    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 0);

    socks.first.send("foo", 4, 0);
    socks.first.send("foo", 4, 0);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 1);

    // No notification for second message.
    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 1);

    // Revert to level-triggered.
    sub.set_events(EpollIn);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 2);

    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 2);

    socks.first.send("foo", 4, 0);
    BOOST_TEST(r.poll(now, 0ms) == 1);
    BOOST_TEST(h->matches == 3);

    BOOST_TEST(r.poll(now, 0ms) == 0);
    BOOST_TEST(h->matches == 3);
}

BOOST_AUTO_TEST_CASE(ReactorHookCase)
{
    int i{0};
    auto fn = [&i](CyclTime) { ++i; };

    Reactor r{1024};

    Hook h{bind(&fn)};
    r.add_hook(h);

    BOOST_TEST(r.poll(CyclTime::now(), 0ms) == 0);
    BOOST_TEST(i == 1);
}

BOOST_AUTO_TEST_SUITE_END()
