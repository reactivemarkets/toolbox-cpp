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

#ifndef TOOLBOX_IPC_MPMCQUEUE_HPP
#define TOOLBOX_IPC_MPMCQUEUE_HPP

#include <toolbox/io/File.hpp>
#include <toolbox/ipc/Mmap.hpp>
#include <toolbox/sys/Limits.hpp>

#include <atomic>
#include <cassert>
#include <memory>

namespace toolbox {
inline namespace ipc {
namespace detail {
inline std::size_t file_size(int fd)
{
    struct stat st;
    os::fstat(fd, st);
    return st.st_size;
}
} // namespace detail

/// MpmcQueue is a bounded MPMC queue implementation based on Dmitry Vyukov's design.
template <typename ValueT>
class MpmcQueue {
    static_assert(std::is_trivially_copyable_v<ValueT>);

  public:
    struct alignas(CacheLineSize) Elem {
        std::int64_t seq;
        ValueT val;
    };
    static_assert(std::is_trivially_copyable_v<Elem>);
    struct alignas(CacheLineSize) Impl {
        // Ensure that read and write positions are in different cache-lines.
        std::int64_t rpos;
        alignas(CacheLineSize) std::int64_t wpos;
        alignas(CacheLineSize) Elem elems[];
    };
    static_assert(std::is_trivially_copyable_v<Impl>);
    static_assert(sizeof(Impl) == 2 * CacheLineSize);
    static_assert(offsetof(Impl, rpos) == 0 * CacheLineSize);
    static_assert(offsetof(Impl, wpos) == 1 * CacheLineSize);
    static_assert(offsetof(Impl, elems) == 2 * CacheLineSize);

    constexpr MpmcQueue(std::nullptr_t = nullptr) noexcept {}
    explicit MpmcQueue(std::size_t capacity)
    : capacity_{next_pow2(capacity)}
    , mask_{capacity_ - 1}
    , mem_map_{os::mmap(nullptr, size(capacity_), PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE,
                        -1, 0)}
    , impl_{static_cast<Impl*>(mem_map_.get().data())}
    {
        assert(capacity >= 2);

        std::memset(impl_, 0, size(capacity_));
        // Initialise sequence numbers.
        for (std::int64_t i{0}; i < static_cast<std::int64_t>(capacity); ++i) {
            __atomic_store_n(&impl_->elems[i].seq, i, __ATOMIC_RELAXED);
        }
    }
    explicit MpmcQueue(FileHandle& fh)
    : capacity_{capacity(detail::file_size(fh.get()))}
    , mask_{capacity_ - 1}
    , mem_map_{os::mmap(nullptr, size(capacity_), PROT_READ | PROT_WRITE, MAP_SHARED, fh.get(), 0)}
    , impl_{static_cast<Impl*>(mem_map_.get().data())}
    {
        if (!is_pow2(capacity_)) {
            throw std::runtime_error{"capacity not a power of two"};
        }
    }
    explicit MpmcQueue(FileHandle&& fh)
    : MpmcQueue{fh}
    {
    }
    /// Opens a file-backed MpmcQueue.
    ///
    /// The mmap() function retains a reference to the file associated with the file descriptor,
    /// so the file can be safely closed once the mapping has been established.
    ///
    /// \param path Path to MpmcQueue file.
    ///
    explicit MpmcQueue(const char* path)
    : MpmcQueue{os::open(path, O_RDWR)}
    {
    }
    ~MpmcQueue() = default;

    // Copy.
    MpmcQueue(const MpmcQueue&) = delete;
    MpmcQueue& operator=(const MpmcQueue&) = delete;

    // Move.
    MpmcQueue(MpmcQueue&& rhs) noexcept
    : capacity_{rhs.capacity_}
    , mask_{rhs.mask_}
    , mem_map_{std::move(rhs.mem_map_)}
    , impl_{rhs.impl_}
    {
        rhs.capacity_ = 0;
        rhs.mask_ = 0;
        rhs.mem_map_ = {};
        rhs.impl_ = nullptr;
    }
    MpmcQueue& operator=(MpmcQueue&& rhs) noexcept
    {
        reset();
        swap(rhs);
        return *this;
    }

    /// Returns true if the queue is empty.
    bool empty() const noexcept
    {
        // Acquire prevents reordering of these loads.
        const auto rpos = __atomic_load_n(&impl_->rpos, __ATOMIC_ACQUIRE);
        const auto wpos = __atomic_load_n(&impl_->wpos, __ATOMIC_RELAXED);
        return rpos == wpos;
    }
    /// Returns true if the queue is full.
    bool full() const noexcept { return size() == capacity_; }
    /// Returns the number of available elements.
    std::size_t available() const noexcept { return capacity_ - size(); }
    /// Returns the maximum number of elements the queue can hold.
    /// I.e. the queue's capacity.
    std::size_t capacity() const noexcept { return capacity_; }
    /// Returns the maximum number of elements the queue can hold.
    /// I.e. the queue's capacity.
    std::size_t max_size() const noexcept { return capacity(); }
    /// Returns true if the number of elements in the queue.
    std::size_t size() const noexcept
    {
        // Acquire prevents reordering of these loads.
        const auto rpos = __atomic_load_n(&impl_->rpos, __ATOMIC_ACQUIRE);
        const auto wpos = __atomic_load_n(&impl_->wpos, __ATOMIC_RELAXED);
        return wpos - rpos;
    }
    void reset(std::nullptr_t = nullptr) noexcept
    {
        // Reverse order.
        impl_ = nullptr;
        mem_map_.reset(nullptr);
        mask_ = 0;
        capacity_ = 0;
    }
    void swap(MpmcQueue& rhs) noexcept
    {
        std::swap(capacity_, rhs.capacity_);
        std::swap(mask_, rhs.mask_);
        mem_map_.swap(rhs.mem_map_);
        std::swap(impl_, rhs.impl_);
    }
    /// Returns false if queue is empty.
    template <typename FnT>
    bool read(FnT fn) noexcept
    {
        static_assert(std::is_nothrow_invocable_v<FnT, ValueT&&>);
        auto rpos = __atomic_load_n(&impl_->rpos, __ATOMIC_RELAXED);
        for (;;) {
            auto& elem = impl_->elems[rpos & mask_];
            const auto seq = __atomic_load_n(&elem.seq, __ATOMIC_ACQUIRE);
            const auto diff = seq - (rpos + 1);
            if (diff == 0) {
                // The compare_exchange_weak function re-reads rpos on failure.
                if (__atomic_compare_exchange_n(&impl_->rpos, &rpos, rpos + 1, true,
                                                __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
                    fn(std::move(elem.val));
                    // Commit.
                    __atomic_store_n(&elem.seq, rpos + capacity_, __ATOMIC_RELEASE);
                    break;
                }
                // Continue.
            } else if (diff < 0) {
                return false;
            } else {
                rpos = __atomic_load_n(&impl_->rpos, __ATOMIC_RELAXED);
            }
        }
        return true;
    }
    /// Returns false if capacity is exceeded.
    template <typename FnT>
    bool write(FnT fn) noexcept
    {
        static_assert(std::is_nothrow_invocable_v<FnT, ValueT&>);
        auto wpos = __atomic_load_n(&impl_->wpos, __ATOMIC_RELAXED);
        for (;;) {
            auto& elem = impl_->elems[wpos & mask_];
            const auto seq = __atomic_load_n(&elem.seq, __ATOMIC_ACQUIRE);
            const auto diff = seq - wpos;
            if (diff == 0) {
                // The compare_exchange_weak function re-reads wpos on failure.
                if (__atomic_compare_exchange_n(&impl_->wpos, &wpos, wpos + 1, true,
                                                __ATOMIC_RELAXED, __ATOMIC_RELAXED)) {
                    fn(elem.val);
                    // Commit.
                    __atomic_store_n(&elem.seq, wpos + 1, __ATOMIC_RELEASE);
                    break;
                }
                // Continue.
            } else if (diff < 0) {
                return false;
            } else {
                wpos = __atomic_load_n(&impl_->wpos, __ATOMIC_RELAXED);
            }
        }
        return true;
    }
    /// Returns false if queue is empty.
    bool pop(ValueT& val) noexcept
    {
        static_assert(std::is_nothrow_move_assignable_v<ValueT>);
        return read([&val](ValueT&& ref) noexcept { val = std::move(ref); });
    }
    /// Returns false if capacity is exceeded.
    bool push(const ValueT& val) noexcept
    {
        static_assert(std::is_nothrow_copy_assignable_v<ValueT>);
        return write([&val](ValueT& ref) noexcept { ref = val; });
    }
    /// Returns false if capacity is exceeded.
    bool push(ValueT&& val) noexcept
    {
        static_assert(std::is_nothrow_move_assignable_v<ValueT>);
        return write([&val](ValueT& ref) noexcept { ref = std::move(val); });
    }

  private:
    static constexpr std::size_t capacity(std::size_t size) noexcept
    {
        return (size - sizeof(Impl)) / sizeof(Elem);
    }
    static constexpr std::size_t size(std::size_t capacity) noexcept
    {
        return sizeof(Impl) + capacity * sizeof(Elem);
    }

    std::uint64_t capacity_{}, mask_{};
    Mmap mem_map_{nullptr};
    Impl* impl_{nullptr};
};

/// Initialise file-based MpmcQueue.
template <typename ValueT>
void new_mpmc_queue(FileHandle& fh, std::size_t capacity)
{
    using Elem = typename MpmcQueue<ValueT>::Elem;
    using Impl = typename MpmcQueue<ValueT>::Impl;

    assert(capacity >= 2);

    capacity = next_pow2(capacity);
    const auto size = sizeof(Impl) + capacity * sizeof(Elem);

    os::ftruncate(fh.get(), size);
    Mmap mem_map{os::mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, fh.get(), 0)};
    auto* const impl = static_cast<Impl*>(mem_map.get().data());

    std::memset(impl, 0, size);
    // Initialise sequence numbers.
    for (std::int64_t i{0}; i < static_cast<std::int64_t>(capacity); ++i) {
        __atomic_store_n(&impl->elems[i].seq, i, __ATOMIC_RELAXED);
    }
}

/// Initialise file-based MpmcQueue.
template <typename ValueT>
void new_mpmc_queue(FileHandle&& fh, std::size_t capacity)
{
    return new_mpmc_queue<ValueT>(fh, capacity);
}

} // namespace ipc
} // namespace toolbox

#endif // TOOLBOX_IPC_MPMCQUEUE_HPP
