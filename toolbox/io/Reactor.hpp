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

#ifndef TOOLBOX_IO_REACTOR_HPP
#define TOOLBOX_IO_REACTOR_HPP

#include <toolbox/io/Epoll.hpp>
#include <toolbox/io/EventFd.hpp>
#include <toolbox/io/Hook.hpp>
#include <toolbox/io/Timer.hpp>
#include <toolbox/io/Waker.hpp>
#include <variant>

namespace toolbox {
inline namespace io {

constexpr Duration NoTimeout{-1};
enum class Priority { High = 0, Low = 1 };
using IoSlot = BasicSlot<void(CyclTime, int, unsigned)>;

enum class ReactorMode {
    // The Reactor may block when polling if a non-zero timeout is specified.
    // Recommended when the Reactor shares a CPU resource with other processes.
    Blocking,

    // The Reactor returns immediately if no events are pending -- i.e. no blocking occurs.
    // Ideal when the Reactor runs on a dedicated (pinned) CPU resource and is polled
    // continuously in a loop.
    Immediate
};

class TOOLBOX_API Reactor : public Waker {
  public:
    using Event = EpollEvent;
    // HookType describes the kind of hook.
    enum class HookType : int {
        // EndOfCycleNoWait hooks are called at the end of the Reactor cycle.
        // The Reactor cycle will not wait for i/o and/or timer events
        // while any of these hooks are installed.
        EndOfCycleNoWait = 1,
        // EndOfEventDispatch hooks are called after all i/o and timer events have been dispatched.
        // These hooks are called, and only if, work done in the cycle is greater than zero.
        // And they are always called before EndOfCycleNoWait hooks.
        EndOfEventDispatch = 2,
    };
    class Handle {
      public:
        Handle(Reactor& reactor, int fd, int sid)
        : reactor_{&reactor}
        , fd_{fd}
        , sid_{sid}
        {
        }
        constexpr Handle(std::nullptr_t = nullptr) noexcept {} // NOLINT(hicpp-explicit-conversions)
        ~Handle() { reset(); }

        // Copy.
        Handle(const Handle&) = delete;
        Handle& operator=(const Handle&) = delete;

        // Move.
        Handle(Handle&& rhs) noexcept
        : reactor_{rhs.reactor_}
        , fd_{rhs.fd_}
        , sid_{rhs.sid_}
        {
            rhs.reactor_ = nullptr;
            rhs.fd_ = -1;
            rhs.sid_ = 0;
        }
        Handle& operator=(Handle&& rhs) noexcept
        {
            reset();
            swap(rhs);
            return *this;
        }
        bool empty() const noexcept { return reactor_ == nullptr; }
        explicit operator bool() const noexcept { return reactor_ != nullptr; }
        auto fd() const noexcept { return fd_; }
        auto sid() const noexcept { return sid_; }

        void reset(std::nullptr_t = nullptr) noexcept
        {
            if (reactor_) {
                reactor_->unsubscribe(fd_, sid_);
                reactor_ = nullptr;
                fd_ = -1;
                sid_ = 0;
            }
        }
        void swap(Handle& rhs) noexcept
        {
            std::swap(reactor_, rhs.reactor_);
            std::swap(fd_, rhs.fd_);
            std::swap(sid_, rhs.sid_);
        }

        /// Modify I/O event subscription.
        void set_events(unsigned events, IoSlot slot, std::error_code& ec) noexcept
        {
            assert(reactor_);
            reactor_->set_events(fd_, sid_, events, slot, ec);
        }
        void set_events(unsigned events, IoSlot slot)
        {
            assert(reactor_);
            reactor_->set_events(fd_, sid_, events, slot);
        }
        void set_events(unsigned events, std::error_code& ec) noexcept
        {
            assert(reactor_);
            reactor_->set_events(fd_, sid_, events, ec);
        }
        void set_events(unsigned events)
        {
            assert(reactor_);
            reactor_->set_events(fd_, sid_, events);
        }

        void set_io_priority(Priority priority)
        {
            assert(reactor_);
            reactor_->set_io_priority(fd_, sid_, priority);
        }

      private:
        Reactor* reactor_{nullptr};
        int fd_{-1}, sid_{0};
    };

    explicit Reactor(ReactorMode mode = ReactorMode::Blocking, std::size_t size_hint = 0);
    ~Reactor() override;

    // Copy.
    Reactor(const Reactor&) = delete;
    Reactor& operator=(const Reactor&) = delete;

    // Move.
    Reactor(Reactor&&) = delete;
    Reactor& operator=(Reactor&&) = delete;

    // clang-format off
    [[nodiscard]] Handle subscribe(int fd, unsigned events, IoSlot slot);

    /// Throws std::bad_alloc only.
    [[nodiscard]] Timer timer(MonoTime expiry, Duration interval, Priority priority, TimerSlot slot)
    {
        return tqs_[static_cast<size_t>(priority)].insert(expiry, interval, slot);
    }
    /// Throws std::bad_alloc only.
    [[nodiscard]] Timer timer(MonoTime expiry, Priority priority, TimerSlot slot)
    {
        return tqs_[static_cast<size_t>(priority)].insert(expiry, slot);
    }
    // clang-format on

    void add_hook(Hook& hook, HookType ht = HookType::EndOfCycleNoWait) noexcept
    {
        switch (ht) {
        case HookType::EndOfCycleNoWait:
            end_of_cycle_no_wait_hooks.push_back(hook);
            break;
        case HookType::EndOfEventDispatch:
            end_of_event_dispatch_hooks_.push_back(hook);
            break;
        }
    }
    /// Poll for I/O and timer events.
    /// The thread-local cycle time is unconditionally updated after the call to epoll() returns.
    /// \param timeout is ignored when immediate mode is used.
    /// Returns the number of events signalled.
    int poll(CyclTime now, Duration timeout = NoTimeout);

    void yield();

    void set_high_priority_poll_threshold(Micros thresh) { priority_io_poll_threshold = thresh; }

  protected:
    /// Thread-safe.
    void do_wakeup() noexcept final;

  private:
    MonoTime next_expiry(MonoTime next) const;

    struct BlockingDevice {
        Epoll epoll;
    };

    struct ImmediateDevice {
        Epoll low_prio_epoll;
        Epoll high_prio_epoll;
    };

    struct Data {
        int sid{};
        unsigned events{};
        IoSlot slot;
        Priority priority = Priority::Low;
    };

    Epoll& get_resident_epoll(Data& data);
    int poll_immediate(ImmediateDevice& dev,CyclTime now);
    int poll_blocking(BlockingDevice& dev,CyclTime now, Duration timeout);
    // dispatch events only for file descriptors with specified priority
    int dispatch(CyclTime now, Event* buf, int size, Priority priority);
    void set_events(int fd, int sid, unsigned events, IoSlot slot, std::error_code& ec) noexcept;
    void set_events(int fd, int sid, unsigned events, IoSlot slot);
    void set_events(int fd, int sid, unsigned events, std::error_code& ec) noexcept;
    void set_events(int fd, int sid, unsigned events);
    void unsubscribe(int fd, int sid) noexcept;
    void set_io_priority(int fd, int sid, Priority priority) noexcept;

    std::variant<BlockingDevice, ImmediateDevice> device_;
    std::vector<Data> data_;
    EventFd notify_{0, EFD_NONBLOCK};
    static_assert(static_cast<int>(Priority::High) == 0);
    static_assert(static_cast<int>(Priority::Low) == 1);
    TimerPool tp_;
    std::array<TimerQueue, 2> tqs_{tp_, tp_};
    HookList end_of_cycle_no_wait_hooks, end_of_event_dispatch_hooks_;
    Micros priority_io_poll_threshold = Micros::max();
    WallTime last_time_priority_io_polled_{};
    int cycle_work_{0};
    bool currently_handling_priority_events_{false};
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_REACTOR_HPP
