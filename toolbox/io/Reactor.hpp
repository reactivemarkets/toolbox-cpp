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

#ifndef TOOLBOX_IO_REACTOR_HPP
#define TOOLBOX_IO_REACTOR_HPP

#include <toolbox/io/Epoll.hpp>
#include <toolbox/io/EventFd.hpp>
#include <toolbox/io/Hook.hpp>
#include <toolbox/io/Waker.hpp>
#include <toolbox/io/Timer.hpp>

namespace toolbox {
inline namespace io {

constexpr Duration NoTimeout{-1};
enum class Priority { High = 0, Low = 1 };

using IoSlot = BasicSlot<CyclTime, int, unsigned>;

class TOOLBOX_API Reactor : public Waker {
  public:
    using Event = EpollEvent;
    class Handle {
      public:
        Handle(Reactor& reactor, int fd, int sid)
        : reactor_{&reactor}
        , fd_{fd}
        , sid_{sid}
        {
        }
        constexpr Handle(std::nullptr_t = nullptr) noexcept {}
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

        /// Modify IO event subscription.
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

      private:
        Reactor* reactor_{nullptr};
        int fd_{-1}, sid_{0};
    };

    explicit Reactor(std::size_t size_hint = 0);
    ~Reactor();

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

    void add_hook(Hook& hook) noexcept { hooks_.push_back(hook); }
    int poll(CyclTime now, Duration timeout = NoTimeout);

  protected:
    /// Thread-safe.
    void do_wakeup() noexcept final;

  private:
    MonoTime next_expiry(MonoTime next) const;

    int dispatch(CyclTime now, Event* buf, int size);
    void set_events(int fd, int sid, unsigned events, IoSlot slot, std::error_code& ec) noexcept;
    void set_events(int fd, int sid, unsigned events, IoSlot slot);
    void set_events(int fd, int sid, unsigned events, std::error_code& ec) noexcept;
    void set_events(int fd, int sid, unsigned events);
    void unsubscribe(int fd, int sid) noexcept;

    struct Data {
        int sid{};
        unsigned events{};
        IoSlot slot;
    };
    Epoll epoll_;
    std::vector<Data> data_;
    EventFd notify_{0, EFD_NONBLOCK};
    static_assert(static_cast<int>(Priority::High) == 0);
    static_assert(static_cast<int>(Priority::Low) == 1);
    TimerPool tp_;
    std::array<TimerQueue, 2> tqs_{tp_, tp_};
    HookList hooks_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_REACTOR_HPP
