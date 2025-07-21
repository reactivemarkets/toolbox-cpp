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
#include <string_view>

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

BOOST_AUTO_TEST_CASE(ReactorSocketPriority)
{
    using namespace literals::chrono_literals;

    Reactor r{1024};

    auto [first_sock, second_sock] = socketpair(UnixStreamProtocol{});

    std::vector<int> fd_process_order;
    auto handler = [&](CyclTime, int fd, unsigned) {
        char buf[5];
        auto rcvd = os::recv(fd, buf, 5, 0);
        BOOST_CHECK_EQUAL(rcvd, 5);

        std::string_view bsv(buf, buf+sizeof(buf));
        BOOST_CHECK_EQUAL(bsv, "Hello");
        fd_process_order.push_back(fd);
    };

    auto sub1 = r.subscribe(*first_sock, EpollIn, bind(&handler));
    auto sub2 = r.subscribe(*second_sock, EpollIn, bind(&handler));

    // test send data from first_sock --> second_sock
    first_sock.send("Hello", 5, 0);
    r.poll(CyclTime::now(), 0ms);
    BOOST_CHECK_EQUAL(fd_process_order.size(), 1);
    BOOST_CHECK_EQUAL(fd_process_order[0], *second_sock);
    fd_process_order.clear();

    // test send data from second_sock --> first_sock
    second_sock.send("Hello", 5, 0);
    r.poll(CyclTime::now(), 0ms);
    BOOST_CHECK_EQUAL(fd_process_order.size(), 1);
    BOOST_CHECK_EQUAL(fd_process_order[0], *first_sock);
    fd_process_order.clear();

    // send data to both sockets -- but from `first_sock` first
    // Set priority of `first_sock` to be High.
    // `second_sock` will receive data first, but because of High priority
    // of `first_sock`, it will be processed first.
    sub1.set_io_priority(Priority::High);
    sub2.set_io_priority(Priority::Low);
    first_sock.send("Hello", 5, 0);
    std::this_thread::sleep_for(100ms);
    second_sock.send("Hello", 5, 0);
    r.poll(CyclTime::now(), 0ms);
    BOOST_CHECK_EQUAL(fd_process_order.size(), 2);
    BOOST_CHECK_EQUAL(fd_process_order[0], *first_sock);
    BOOST_CHECK_EQUAL(fd_process_order[1], *second_sock);
    fd_process_order.clear();

    // Do same again, except switch priorities.
    sub1.set_io_priority(Priority::Low);
    sub2.set_io_priority(Priority::High);
    first_sock.send("Hello", 5, 0);
    std::this_thread::sleep_for(100ms);
    second_sock.send("Hello", 5, 0);
    r.poll(CyclTime::now(), 0ms);
    BOOST_CHECK_EQUAL(fd_process_order.size(), 2);
    BOOST_CHECK_EQUAL(fd_process_order[0], *second_sock);
    BOOST_CHECK_EQUAL(fd_process_order[1], *first_sock);
    fd_process_order.clear();
}

BOOST_AUTO_TEST_CASE(ReactorHighPriorityYield)
{
    using namespace literals::chrono_literals;

    Reactor r{1024};
    r.set_high_priority_poll_threshold(50us);

    // h0, h1 will be high priority
    // l0, l1 will be low priority
    auto [h0, h1] = socketpair(UnixStreamProtocol{});
    auto [l0, l1] = socketpair(UnixStreamProtocol{});

    auto send_data_to = [&](IoSock& sock) {
        IoSock& corresponding = (sock == h0) ? h1
                              : (sock == h1) ? h0
                              : (sock == l0) ? l1
                              : l0;
        corresponding.send("Hello", 5, 0);
    };

    // this will track entry/exit of handlers of each of the sockets
    struct Audit {
        enum { Entry, Exit } what;
        int fd;
        bool operator==(const Audit& o) const = default;
    };
    std::vector<Audit> audit_trail;

    // each invocation will take ~100ms
    auto spin_and_yield_periodically = [&r]() {
        WallTime now = WallClock::now();
        WallTime end = now + 100ms;
        while (now < end) {
            // wait for 10us, then yield
            auto next_stop = now + 10us;
            while (now < next_stop) {
                now = WallClock::now();
            }
            r.yield();
        };
    };

    // reads data from fd, then pretends to do work (by spinning),
    // yielding periodically to the reactor.
    auto high_handler = [&](CyclTime, int fd, unsigned) {
        audit_trail.push_back({.what = Audit::Entry, .fd = fd});
        char buf[5];
        auto rcvd = os::recv(fd, buf, 5, 0);
        BOOST_CHECK_EQUAL(rcvd, 5);
        spin_and_yield_periodically();
        audit_trail.push_back({.what = Audit::Exit, .fd = fd});
    };

    // reads data from fd, sends message to h0/h1, then pretends to do work (by spinning),
    // yielding periodically to the reactor. In those yields, the messages sent to the
    // high priority sockets should be processed.
    auto low_handler = [&]<bool H0, bool H1>(CyclTime, int fd, unsigned) {
        audit_trail.push_back({.what = Audit::Entry, .fd = fd});

        char buf[5];
        auto rcvd = os::recv(fd, buf, 5, 0);
        BOOST_CHECK_EQUAL(rcvd, 5);

        // send data to both high priority sockets
        if constexpr (H0) {
            send_data_to(h0);
        }
        if constexpr (H1) {
            send_data_to(h1);
        }

        spin_and_yield_periodically();
        audit_trail.push_back({.what = Audit::Exit, .fd = fd});
    };

    auto l0_handler = [&low_handler](CyclTime ct, int fd, unsigned int e) {
        low_handler.template operator()<true, true>(ct, fd, e);
    };

    auto l1_handler = [&low_handler](CyclTime ct, int fd, unsigned int e) {
        low_handler.template operator()<false, true>(ct, fd, e);
    };

    auto sub_l0 = r.subscribe(*l0, EpollIn, bind(&l0_handler));
    auto sub_l1 = r.subscribe(*l1, EpollIn, bind(&l1_handler));
    auto sub_h0 = r.subscribe(*h0, EpollIn, bind(&high_handler));
    auto sub_h1 = r.subscribe(*h1, EpollIn, bind(&high_handler));

    sub_h0.set_io_priority(Priority::High);
    sub_h1.set_io_priority(Priority::High);

    // start off with l0, l1, h0 having data to read
    send_data_to(l0);
    send_data_to(l1);
    send_data_to(h0);

    r.poll(CyclTime::now(), 0ms);

    std::vector<Audit> valid_seq1 = {
        // high priority always first
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // AT THIS POINT l0/l1 handlers could be executed in any order
        // Lets assume l0 chosen first [choices so far: (l0)]
        Audit{Audit::Entry, *l0}, // l0 handler sends data to h0 and h1

        // Reactor will poll on high priority sockets when l0 handler yields
        // AT THIS POINT h0/h1 handlers could be executed in any order
        // Lets assume h0 is chosen first [choices so far: (l0, h0)]
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},

        // control returned back to l0 handler
        Audit{Audit::Exit, *l0},

        // Now, l1 handler executes
        Audit{Audit::Entry, *l1}, // l1 handler sends data to h1
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Exit, *l1}, // l1 handler sends data to h1
    };

    std::vector<Audit> valid_seq2 = {
        // high priority always first
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // AT THIS POINT l0/l1 handlers could be executed in any order
        // Lets assume l0 chosen first [choices so far: (l0)]
        Audit{Audit::Entry, *l0}, // l0 handler sends data to h0 and h1

        // Reactor will poll on high priority sockets when l0 handler yields
        // AT THIS POINT h0/h1 handlers could be executed in any order
        // Lets assume h1 chosen first [choices so far: (l0, h1)]
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // control returned back to l0 handler
        Audit{Audit::Exit, *l0},

        // Now, l1 handler executes
        Audit{Audit::Entry, *l1}, // l1 handler sends data to h1
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Exit, *l1}, // l1 handler sends data to h1
    };

    std::vector<Audit> valid_seq3 = {
        // high priority always first
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // AT THIS POINT l0/l1 handlers could be executed in any order
        // Lets assume l0 chosen first [choices so far: (l1)]
        Audit{Audit::Entry, *l1}, // l1 handler sends data to h1
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Exit, *l1},

        // Now, l0 handler executes
        Audit{Audit::Entry, *l0}, // l0 handler sends data to h0 and h1
        // Reactor will poll on high priority sockets when l0 handler yields
        // AT THIS POINT h0/h1 handlers could be executed in any order
        // Lets assume h0 chosen first [choices so far: (l1, h0)]
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},

        // control returned back to l0 handler
        Audit{Audit::Exit, *l0},
    };

    std::vector<Audit> valid_seq4 = {
        // high priority always first
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // AT THIS POINT l0/l1 handlers could be executed in any order
        // Lets assume l0 chosen first [choices so far: (l1)]
        Audit{Audit::Entry, *l1}, // l1 handler sends data to h1
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Exit, *l1},

        // Now, l0 handler executes
        Audit{Audit::Entry, *l0}, // l0 handler sends data to h0 and h1
        // Reactor will poll on high priority sockets when l0 handler yields
        // AT THIS POINT h0/h1 handlers could be executed in any order
        // Lets assume h1 chosen first [choices so far: (l1, h1)]
        Audit{Audit::Entry, *h1},
        Audit{Audit::Exit, *h1},
        Audit{Audit::Entry, *h0},
        Audit{Audit::Exit, *h0},

        // control returned back to l0 handler
        Audit{Audit::Exit, *l0},
    };

    bool trail_matches =  (audit_trail == valid_seq1)
                       || (audit_trail == valid_seq2)
                       || (audit_trail == valid_seq3)
                       || (audit_trail == valid_seq4);
    BOOST_CHECK_EQUAL(trail_matches, true);
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
