// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#ifndef TOOLBOX_IO_EPOLLREACTOR_HPP
#define TOOLBOX_IO_EPOLLREACTOR_HPP

#include <toolbox/io/EventFd.hpp>
#include <toolbox/io/Muxer.hpp>
#include <toolbox/io/Reactor.hpp>

namespace toolbox {
inline namespace io {

class TOOLBOX_API EpollReactor : public Reactor {
  public:
    using Event = typename EpollMuxer::Event;

    explicit EpollReactor(std::size_t size_hint = 1024);
    ~EpollReactor() override;

    // Copy.
    EpollReactor(const EpollReactor&) = delete;
    EpollReactor& operator=(const EpollReactor&) = delete;

    // Move.
    EpollReactor(EpollReactor&&) = delete;
    EpollReactor& operator=(EpollReactor&&) = delete;

  protected:
    /// Thread-safe.
    void do_interrupt() noexcept override;

    Handle do_subscribe(int fd, unsigned events, IoSlot slot) override;
    void do_unsubscribe(int fd, int sid) noexcept override;

    void do_set_events(int fd, int sid, unsigned events, IoSlot slot,
                       std::error_code& ec) noexcept override;
    void do_set_events(int fd, int sid, unsigned events, IoSlot slot) override;
    void do_set_events(int fd, int sid, unsigned events, std::error_code& ec) noexcept override;
    void do_set_events(int fd, int sid, unsigned events) override;

    /// Throws std::bad_alloc only.
    Timer do_timer(MonoTime expiry, Duration interval, Priority priority, TimerSlot slot) override;
    /// Throws std::bad_alloc only.
    Timer do_timer(MonoTime expiry, Priority priority, TimerSlot slot) override;

    int do_poll(CyclTime now, Duration timeout) override;

  private:
    MonoTime next_expiry(MonoTime next) const;
    int dispatch(CyclTime now, Event* buf, int size);

    struct Data {
        int sid{};
        unsigned events{};
        IoSlot slot;
    };
    EpollMuxer mux_;
    std::vector<Data> data_;
    EventFd efd_{0, EFD_NONBLOCK};
    static_assert(static_cast<int>(Priority::High) == 0);
    static_assert(static_cast<int>(Priority::Low) == 1);
    TimerPool tp_;
    std::array<TimerQueue, 2> tqs_{tp_, tp_};
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_EPOLLREACTOR_HPP
