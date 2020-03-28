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

#include "Timer.hpp"

#include <toolbox/sys/Log.hpp>

#include <algorithm>
#include <string>

namespace toolbox {
inline namespace io {
using namespace std;
namespace {
// Number of entries per 4K slab, assuming that malloc overhead is no more than 16 bytes.
constexpr size_t Overhead = 16;
constexpr size_t PageSize = 4096;
constexpr size_t SlabSize = (PageSize - Overhead) / sizeof(Timer);

bool is_after(const Timer& lhs, const Timer& rhs)
{
    return lhs.expiry() > rhs.expiry();
}

} // namespace

Timer::Impl* TimerPool::alloc(MonoTime expiry, Duration interval, TimerSlot slot)
{
    Timer::Impl* impl;

    if (free_) {

        // Pop next free timer from stack.
        impl = free_;
        free_ = free_->next;

    } else {

        // Add new slab of timers to stack.
        SlabPtr slab{new Timer::Impl[SlabSize]};
        impl = &slab[0];

        for (size_t i{1}; i < SlabSize; ++i) {
            slab[i].next = free_;
            free_ = &slab[i];
        }
        slabs_.push_back(move(slab));
    }

    return impl;
}

Timer TimerQueue::insert(MonoTime expiry, Duration interval, TimerSlot slot)
{
    assert(slot);

    heap_.reserve(heap_.size() + 1);
    const auto tmr = alloc(expiry, interval, slot);

    // Cannot fail.
    heap_.push_back(tmr);
    push_heap(heap_.begin(), heap_.end(), is_after);

    return tmr;
}

int TimerQueue::dispatch(CyclTime now)
{
    int work{};
    while (!heap_.empty()) {

        // If not pending, then must have been cancelled.
        if (!heap_.front().pending()) {
            pop();
            --cancelled_;
            assert(cancelled_ >= 0);
        } else if (heap_.front().expiry() <= now.mono_time()) {
            expire(now);
            ++work;
        } else {
            break;
        }
    }
    gc();
    return work;
}

Timer TimerQueue::alloc(MonoTime expiry, Duration interval, TimerSlot slot)
{
    Timer::Impl* impl{pool_.alloc(expiry, interval, slot)};

    impl->tq = this;
    impl->ref_count = 1;
    impl->id = ++max_id_;
    impl->expiry = expiry;
    impl->interval = interval;
    impl->slot = slot;

    return Timer{impl};
}

void TimerQueue::cancel() noexcept
{
    ++cancelled_;

    // Ensure that a pending timer is at the front of the queue.
    // If not pending, then must have been cancelled.
    while (!heap_.empty() && !heap_.front().pending()) {
        pop();
        --cancelled_;
        assert(cancelled_ >= 0);
    }
    gc();
}

void TimerQueue::expire(CyclTime now)
{
    // Pop timer.
    auto tmr = pop();
    assert(tmr.pending());
    try {
        // Notify user.
        tmr.slot().invoke(now, tmr);
    } catch (const std::exception& e) {
        TOOLBOX_ERROR << "error handling timer event: " << e.what();
    }

    // If timer was not cancelled during the callback.
    if (tmr.pending()) {

        // If periodic timer.
        if (tmr.interval().count() > 0) {

            // Add interval to expiry, while ensuring that next expiry is always in the future.
            tmr.set_expiry(max(tmr.expiry() + tmr.interval(), now.mono_time() + 1ns));

            // Reschedule popped timer.
            heap_.push_back(tmr);
            push_heap(heap_.begin(), heap_.end(), is_after);

        } else {

            // Free handler for non-repeating timer.
            tmr.slot().reset();
        }
    }
}

void TimerQueue::gc() noexcept
{
    // Garbage collect if more than half of the timers have been cancelled.
    if (cancelled_ > static_cast<int>(heap_.size() >> 1)) {
        const auto it
            = remove_if(heap_.begin(), heap_.end(), [](const auto& tmr) { return !tmr.pending(); });
        heap_.erase(it, heap_.end());
        make_heap(heap_.begin(), heap_.end(), is_after);
        cancelled_ = 0;
    }
}

Timer TimerQueue::pop() noexcept
{
    auto tmr = heap_.front();
    pop_heap(heap_.begin(), heap_.end(), is_after);
    heap_.pop_back();
    return tmr;
}

void intrusive_ptr_release(Timer::Impl* impl) noexcept
{
    --impl->ref_count;
    if (impl->ref_count == 1) {
        // Cancel pending if only one reference remains. If only one reference remains after
        // decrementing the counter, and the timer is still pending, then the final reference must
        // be held within the internal timer queue, which means that no more references exist
        // outside of the timer queue.
        if (impl->slot) {
            impl->slot.reset();
            impl->tq->cancel();
        }
    } else if (impl->ref_count == 0) {
        impl->tq->pool_.dealloc(impl);
    }
}

} // namespace io
} // namespace toolbox
