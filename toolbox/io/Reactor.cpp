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

#include <toolbox/io/TimerFd.hpp>
#include <toolbox/sys/Log.hpp>
#include <toolbox/sys/Trace.hpp>

namespace toolbox {
inline namespace io {
using namespace std;
namespace {
constexpr size_t MaxEvents{128};

int dispatch_low_priority_timers(CyclTime now, TimerQueue& tq, bool idle_cycle)
{
    int work_done = 0;
    if (idle_cycle) {
        work_done = tq.dispatch(now, 1);
    }
    else if (!tq.empty()) {
        // actively execute low priority timers if they've been delayed by 100ms or more.
        if ((now.mono_time() - tq.front().expiry()) > 100ms) {
            work_done = tq.dispatch(now, 1);
        }
    }
    return work_done;
}
} // namespace

Reactor::Reactor(std::size_t size_hint)
{
    const auto notify = notify_.fd();
    data_.resize(max<size_t>(notify + 1, size_hint));
    epoll_.add(notify, 0, EpollIn);
}

Reactor::~Reactor()
{
    epoll_.del(notify_.fd());
}

Reactor::Handle Reactor::subscribe(int fd, unsigned events, IoSlot slot)
{
    assert(fd >= 0);
    assert(slot);
    if (fd >= static_cast<int>(data_.size())) {
        data_.resize(fd + 1);
    }
    auto& ref = data_[fd];
    epoll_.add(fd, ++ref.sid, events);
    ref.events = events;
    ref.slot = slot;
    ref.priority = Priority::Low;
    return {*this, fd, ref.sid};
}

int Reactor::poll(CyclTime now, Duration timeout)
{
    enum { High = 0, Low = 1 };
    using namespace chrono;

    // If timeout is zero then the wait_until time should also be zero to signify no wait.
    MonoTime wait_until{};
    if (!is_zero(timeout) && end_of_cycle_no_wait_hooks.empty()) {
        const MonoTime next
            = next_expiry(timeout == NoTimeout ? MonoClock::max() : now.mono_time() + timeout);
        if (next > now.mono_time()) {
            wait_until = next;
        }
    }
    // TODO: consider using a dynamic buffer that scales with increased demand.
    Event buf[MaxEvents];

    int n;
    error_code ec;
    if (wait_until < MonoClock::max()) {
        // The wait function will not block if time is zero.
        n = epoll_.wait(buf, MaxEvents, wait_until, ec);
    } else {
        // Block indefinitely.
        n = epoll_.wait(buf, MaxEvents, ec);
    }
    // Update cycle time after epoll() returns.
    now = CyclTime::now();
    last_time_priority_io_polled_ = now.wall_time();

    if (ec) {
        if (ec.value() != EINTR) {
            throw system_error{ec};
        }
        return 0;
    }
    cycle_work_ = 0;
    TOOLBOX_PROBE_SCOPED(reactor, dispatch, cycle_work_);
    // High priority timers.
    cycle_work_ = tqs_[High].dispatch(now);
    // I/O events.
    cycle_work_ += dispatch(now, buf, n, Priority::High);
    cycle_work_ += dispatch(now, buf, n, Priority::Low);
    // Low priority timers (typically only dispatched during empty cycles).
    cycle_work_ += dispatch_low_priority_timers(now, tqs_[Low], cycle_work_ == 0);
    // End of cycle hooks.
    if (cycle_work_ > 0) {
        io::dispatch(now, end_of_event_dispatch_hooks_);
    }
    io::dispatch(now, end_of_cycle_no_wait_hooks);
    return cycle_work_;
}

void Reactor::do_wakeup() noexcept
{
    // Best effort.
    std::error_code ec;
    notify_.write(1, ec);
}

MonoTime Reactor::next_expiry(MonoTime next) const
{
    enum { High = 0, Low = 1 };
    using namespace chrono;
    {
        const auto& tq = tqs_[High];
        if (!tq.empty()) {
            // Duration until next expiry. Mitigate scheduler latency by preempting the
            // high-priority timer and busy-waiting for 200us ahead of timer expiry.
            next = min(next, tq.front().expiry() - 200us);
        }
    }
    {
        const auto& tq = tqs_[Low];
        if (!tq.empty()) {
            // Duration until next expiry.
            next = min(next, tq.front().expiry());
        }
    }
    return next;
}

void Reactor::yield()
{
    if (currently_handling_priority_events_) [[unlikely]] {
        return;
    }

    if (priority_io_poll_threshold == Micros::max()) {
        return;
    }

    WallTime now = WallClock::now();
    if (now - last_time_priority_io_polled_ > priority_io_poll_threshold) {
        last_time_priority_io_polled_ = now;

        error_code ec;
        Event buf[MaxEvents];

        int n = epoll_.wait(buf, MaxEvents, MonoTime{}, ec);

        if (ec) {
            if (ec.value() != EINTR) {
                throw system_error{ec};
            }
            return;
        }

        cycle_work_ += dispatch(CyclTime::current(), buf, n, Priority::High);
    }
}

int Reactor::dispatch(CyclTime now, Event* buf, int size, Priority priority)
{
    if (priority == Priority::High) {
        assert(!currently_handling_priority_events_);
        currently_handling_priority_events_ = true;
    }
    const auto reset_flag = make_finally([this]() noexcept {
        currently_handling_priority_events_ = false;
    });

    int work{0};
    for (int i{0}; i < size; ++i) {

        auto& ev = buf[i];
        const auto fd = Epoll::fd(ev);
        const auto& ref = data_[fd];

        if (ref.priority != priority) {
            continue;
        }

        if (fd == notify_.fd()) {
            notify_.read();
            continue;
        }

        if (!ref.slot) {
            // Ignore timerfd.
            continue;
        }

        const auto sid = Epoll::sid(ev);
        // Skip this socket if it was modified after the call to wait().
        if (ref.sid > sid) {
            continue;
        }
        // Apply the interest events to filter-out any events that the user may have removed from
        // the events since the call to wait() was made. This would typically happen via a reentrant
        // call into the reactor from an event-handler. N.B. EpollErr and EpollHup are always
        // reported if they occur, regardless of whether they are specified in events.
        const auto events = ev.events & (ref.events | EpollErr | EpollHup);
        if (!events) {
            continue;
        }

        try {
            ref.slot(now, fd, events);
        } catch (const std::exception& e) {
            TOOLBOX_ERROR << "exception in i/o event handler: " << e.what();
        }
        ++work;
    }
    return work;
}

void Reactor::set_events(int fd, int sid, unsigned events, IoSlot slot, error_code& ec) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        if (ref.events != events) {
            epoll_.mod(fd, sid, events, ec);
            if (ec) {
                return;
            }
            ref.events = events;
        }
        ref.slot = slot;
    }
}

void Reactor::set_events(int fd, int sid, unsigned events, IoSlot slot)
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        if (ref.events != events) {
            epoll_.mod(fd, sid, events);
            ref.events = events;
        }
        ref.slot = slot;
    }
}

void Reactor::set_events(int fd, int sid, unsigned events, error_code& ec) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.events != events) {
        epoll_.mod(fd, sid, events, ec);
        if (ec) {
            return;
        }
        ref.events = events;
    }
}

void Reactor::set_events(int fd, int sid, unsigned events)
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.events != events) {
        epoll_.mod(fd, sid, events);
        ref.events = events;
    }
}

void Reactor::unsubscribe(int fd, int sid) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        epoll_.del(fd);
        ref.events = 0;
        ref.slot.reset();
        ref.priority = Priority::Low;
    }
}

void Reactor::set_io_priority(int fd, int sid, Priority priority) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.priority != priority) {
        ref.priority = priority;
    }
}

} // namespace io
} // namespace toolbox
