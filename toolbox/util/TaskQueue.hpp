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

#ifndef TOOLBOX_UTIL_TASKQUEUE_HPP
#define TOOLBOX_UTIL_TASKQUEUE_HPP

#include <condition_variable>
#include <vector>

namespace toolbox {
inline namespace util {

/// A vector-based task queue for use in multi-threaded, producer-consumer components.
template <typename TaskT>
class TaskQueue final {
    using Lock = std::unique_lock<std::mutex>;

  public:
    TaskQueue() = default;
    ~TaskQueue() = default;

    // Copy.
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    // Move.
    TaskQueue(TaskQueue&&) = delete;
    TaskQueue& operator=(TaskQueue&&) = delete;

    /// Wait for and execute the next task in the queue.
    ///
    /// \return false if the queue was stopped.
    template <typename FnT>
    bool run(FnT fn)
    {
        TaskT task{};

        Lock lock{mutex_};
        // Predicate returns false if the waiting should be continued.
        const auto pred = [this] { return i_ < queue_.size() || stop_; };
        cond_.wait(lock, pred);
        if (queue_.empty() && stop_) {
            return false;
        }
        task = std::move(queue_[i_++]);
        // Clear the queue when the read index reaches the end of the queue.
        if (i_ == queue_.size()) {
            i_ = 0;
            queue_.clear();
        }
        lock.unlock();

        fn(std::move(task));
        return true;
    }
    /// Interrupt and exit any inprogress call to run().
    void stop()
    {
        Lock lock{mutex_};
        stop_ = true;
        // Unlock mutex before waking-up to avoid contention.
        lock.unlock();
        cond_.notify_all();
    }
    /// Clear task queue.
    void clear() noexcept
    {
        Lock lock{mutex_};
        i_ = 0;
        queue_.clear();
    }
    /// Push task onto the task queue.
    bool push(TaskT&& task) noexcept
    {
        Lock lock{mutex_};
        if (stop_) {
            return false;
        }
        queue_.push_back(std::move(task));
        lock.unlock();
        cond_.notify_one();
        return true;
    }

  private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    /// Index to the next read position.
    std::size_t i_{0};
    std::vector<TaskT> queue_;
    bool stop_{false};
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TASKQUEUE_HPP
