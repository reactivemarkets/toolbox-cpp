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
#include <toolbox/util/Variant.hpp>

#include <variant>

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

Reactor::Reactor(ReactorMode mode, std::size_t size_hint)
{
    const auto notify = notify_.fd();
    data_.resize(max<size_t>(notify + 1, size_hint));

    if (mode == ReactorMode::Immediate) {
        device_.emplace<ImmediateDevice>();
        // notify fd does not need to be added to ImmediateDevice's epoll set.
        // This is because the notify fd only exists to allow the Reactor to be woken
        // up -- however, in immediate mode the reactor never blocks.
    } else {
        auto& dev = device_.emplace<BlockingDevice>();
        dev.epoll.add(notify, 0, EpollIn);
    }
}

Reactor::~Reactor()
{
    std::visit(overloaded{
        [this](ImmediateDevice& dev) { dev.low_prio_epoll.del(notify_.fd()); },
        [this](BlockingDevice& dev) { dev.epoll.del(notify_.fd()); }
    }, device_);
}

Reactor::Handle Reactor::subscribe(int fd, unsigned events, IoSlot slot)
{
    assert(fd >= 0);
    assert(slot);
    if (fd >= static_cast<int>(data_.size())) {
        data_.resize(fd + 1);
    }
    auto& ref = data_[fd];
    std::visit(overloaded{
        [&](ImmediateDevice& dev) { dev.low_prio_epoll.add(fd, ++ref.sid, events); },
        [&](BlockingDevice& dev) { dev.epoll.add(fd, ++ref.sid, events); }
    }, device_);
    ref.events = events;
    ref.slot = slot;
    ref.priority = Priority::Low;
    return {*this, fd, ref.sid};
}

int Reactor::poll_blocking(BlockingDevice& dev, CyclTime now, Duration timeout)
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
        n = dev.epoll.wait(buf, MaxEvents, wait_until, ec);
    } else {
        // Block indefinitely.
        n = dev.epoll.wait(buf, MaxEvents, ec);
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
    TOOLBOX_PROBE_SCOPED(reactor, dispatch_blocking, cycle_work_);
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

int Reactor::poll_immediate(ImmediateDevice& dev, CyclTime now)
{
    enum { High = 0, Low = 1 };

    auto dispatch_io = [&]() -> int {
        Event buf[MaxEvents];
        error_code ec;

        // high priority IO
        last_time_priority_io_polled_ = now.wall_time();
        int hn = dev.high_prio_epoll.wait(buf, MaxEvents, MonoTime{}, ec);
        if (ec) {
            if (ec.value() != EINTR) {
                throw system_error{ec};
            }
            return 0;
        }
        dispatch(now, buf, hn, Priority::High);

        // low priority IO
        int ln = dev.low_prio_epoll.wait(buf, MaxEvents, MonoTime{}, ec);
        if (ec) {
            if (ec.value() != EINTR) {
                throw system_error{ec};
            }
            return hn;
        }
        dispatch(now, buf, ln, Priority::Low);

        return hn + ln;
    };

    cycle_work_ = 0;
    TOOLBOX_PROBE_SCOPED(reactor, dispatch_immediate, cycle_work_);
    // High priority timers.
    cycle_work_ = tqs_[High].dispatch(now);
    // I/O events.
    cycle_work_ += dispatch_io();
    // Low priority timers (typically only dispatched during empty cycles).
    cycle_work_ += dispatch_low_priority_timers(now, tqs_[Low], cycle_work_ == 0);
    // End of cycle hooks.
    if (cycle_work_ > 0) {
        io::dispatch(now, end_of_event_dispatch_hooks_);
    }
    io::dispatch(now, end_of_cycle_no_wait_hooks);
    return cycle_work_;
}

int Reactor::poll(CyclTime now, Duration timeout)
{
    return std::visit(overloaded{
        [&](ImmediateDevice& dev) { return poll_immediate(dev, now); },
        [&](BlockingDevice& dev) { return poll_blocking(dev, now, timeout); }
    }, device_);
}

void Reactor::do_wakeup() noexcept
{
    if (std::holds_alternative<BlockingDevice>(device_)) {
        // Best effort.
        std::error_code ec;
        notify_.write(1, ec);
    }
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

        Epoll& epoll = std::visit(overloaded{
            [&](ImmediateDevice& dev) -> Epoll& { return dev.high_prio_epoll; },
            [&](BlockingDevice& dev) -> Epoll& { return dev.epoll; }
        }, device_);

        int n = epoll.wait(buf, MaxEvents, MonoTime{}, ec);

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

Epoll& Reactor::get_resident_epoll(Data& data)
{
    return std::visit(overloaded{
        [&](ImmediateDevice& dev) -> Epoll& {
            return (data.priority == Priority::High) ? dev.high_prio_epoll
                                                     : dev.low_prio_epoll;
            },
        [&](BlockingDevice& dev) -> Epoll& { return dev.epoll; }
    }, device_);
}

void Reactor::set_events(int fd, int sid, unsigned events, IoSlot slot, error_code& ec) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        if (ref.events != events) {
            Epoll& epoll = get_resident_epoll(ref);
            epoll.mod(fd, sid, events, ec);
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
            Epoll& epoll = get_resident_epoll(ref);
            epoll.mod(fd, sid, events);
            ref.events = events;
        }
        ref.slot = slot;
    }
}

void Reactor::set_events(int fd, int sid, unsigned events, error_code& ec) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.events != events) {
        Epoll& epoll = get_resident_epoll(ref);
        epoll.mod(fd, sid, events, ec);
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
        Epoll& epoll = get_resident_epoll(ref);
        epoll.mod(fd, sid, events);
        ref.events = events;
    }
}

void Reactor::unsubscribe(int fd, int sid) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid) {
        Epoll& epoll = get_resident_epoll(ref);
        epoll.del(fd);
        ref.events = 0;
        ref.slot.reset();
        ref.priority = Priority::Low;
    }
}

void Reactor::set_io_priority(int fd, int sid, Priority priority) noexcept
{
    auto& ref = data_[fd];
    if (ref.sid == sid && ref.priority != priority) {
        std::visit(overloaded{
            [&](ImmediateDevice& dev) {
                if (ref.priority == Priority::Low) {
                    dev.low_prio_epoll.del(fd);
                    dev.high_prio_epoll.add(fd, sid, ref.events);
                } else {
                    dev.high_prio_epoll.del(fd);
                    dev.low_prio_epoll.add(fd, sid, ref.events);
                }
            },
            [&](BlockingDevice& /*dev*/) {}
        }, device_);
        ref.priority = priority;
    }
}

} // namespace io
} // namespace toolbox
