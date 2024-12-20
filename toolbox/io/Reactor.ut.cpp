// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2021 Reactive Markets Limited
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
#include <thread>

using namespace std;
using namespace toolbox;

namespace {

struct TestHandler : RefCount<TestHandler, ThreadUnsafePolicy> {
    void on_input(CyclTime /*now*/, int fd, unsigned /*events*/)
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
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 0);

    socks.first.send("foo", 4, 0);
    socks.first.send("foo", 4, 0);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 1);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 2);

    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 2);

    socks.first.send("foo", 4, 0);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 3);

    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 3);
}

BOOST_AUTO_TEST_CASE(ReactorEdgeCase)
{
    using namespace literals::chrono_literals;

    Reactor r{1024};
    auto h = make_intrusive<TestHandler>();

    auto socks = socketpair(UnixStreamProtocol{});
    auto sub = r.subscribe(*socks.second, EpollIn | EpollEt, bind<&TestHandler::on_input>(h.get()));

    const auto now = CyclTime::now();
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 0);

    socks.first.send("foo", 4, 0);
    socks.first.send("foo", 4, 0);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 1);

    // No notification for second message.
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 1);

    // Revert to level-triggered.
    sub.set_events(EpollIn);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 2);

    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 2);

    socks.first.send("foo", 4, 0);
    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 1);
    BOOST_CHECK_EQUAL(h->matches, 3);

    BOOST_CHECK_EQUAL(r.poll(now, 0ms), 0);
    BOOST_CHECK_EQUAL(h->matches, 3);
}

BOOST_AUTO_TEST_CASE(ReactorHookCase)
{
    int i{0};
    auto fn = [&i](CyclTime) { ++i; };

    Reactor r{1024};

    Hook h{bind(&fn)};
    r.add_hook(h);

    BOOST_CHECK_EQUAL(r.poll(CyclTime::now(), 0ms), 0);
    BOOST_CHECK_EQUAL(i, 1);
}

BOOST_AUTO_TEST_CASE(ReactorLowPriorityProgress)
{
    Reactor r{1024};

    struct counter {
        int invocation_count{0};
        void operator()(CyclTime, Timer&) { ++invocation_count; }
    };
    counter lpc;
    counter hpc;

    auto now = CyclTime::now();

    // schedule 2 low priority timers for immediate execution
    Timer lpts[] = {
        r.timer(now.mono_time(), Priority::Low, bind<&counter::operator()>(&lpc)),
        r.timer(now.mono_time(), Priority::Low, bind<&counter::operator()>(&lpc)),
    };

    Timer hpt;

    int num_of_times_polled = 0;
    const auto end = now.mono_time() + 95ms;

    // using 95ms instead of 100ms, because Reactor::poll internally
    // uses own CyclTime, not the one we pass to it. 

    while (now.mono_time() < end) {

        // schedule a high priority timer for immediate execution
        hpt = r.timer(now.mono_time(), Priority::High, bind<&counter::operator()>(&hpc));

        // low priority timers won't be executed because it will be a busy cycle
        // due to high priority timer that is due execution.
        r.poll(now, 0s);

        BOOST_CHECK_EQUAL(hpc.invocation_count, ++num_of_times_polled);
        BOOST_CHECK_EQUAL(lpc.invocation_count, 0);

        now = CyclTime::now();
    }

    std::this_thread::sleep_for(10ms);

    // at this point, both low priority timers are delayed by >100ms.
    // they will now activately executed (one per cycle) even if cycle is busy.
    for (int i = 0; i < 2; i++) {
        hpt = r.timer(now.mono_time(), Priority::High, bind<&counter::operator()>(&hpc));

        r.poll(now, 0s);

        BOOST_CHECK_EQUAL(hpc.invocation_count, ++num_of_times_polled);
        BOOST_CHECK_EQUAL(lpc.invocation_count, i+1);

        now = CyclTime::now();
    }
}

BOOST_AUTO_TEST_SUITE_END()
