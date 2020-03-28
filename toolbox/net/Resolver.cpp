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

#include "Resolver.hpp"

#include <toolbox/net/Endpoint.hpp>

namespace toolbox {
inline namespace net {

int Resolver::run(Duration timeout)
{
    constexpr size_t BatchSize = 16;

    Lock lock{mutex_};
    // Predicate returns ​false if the waiting should be continued.
    const auto pred = [this] { return !this->queue_.empty() || stop_; };
    // Returns false if predicate was false after timeout.
    if (!cond_.wait_for(lock, timeout, pred)) {
        return 0;
    } else if (stop_) {
        // This will unblock waiters by throwing a "broken promise" exception.
        queue_.clear();
        return -1;
    }
    // Copy batch of tasks to temporary buffer.
    std::array<Task, BatchSize> tasks{};

    const auto n = std::min(queue_.size(), BatchSize);
    for (std::size_t i{0}; i < n; ++i) {
        // Copy is disabled for unique_ptr<>, so use swap() instead.
        queue_[i].swap(tasks[i]);
    }
    queue_.erase(queue_.begin(), queue_.begin() + n);
    lock.unlock();

    // Execute tasks after lock is released.
    for (std::size_t i{0}; i < n; ++i) {
        tasks[i]();
    }
    return n;
}

void Resolver::stop()
{
    Lock lock{mutex_};
    stop_ = true;
    // Unlock mutex before notifying to avoid contention.
    lock.unlock();
    cond_.notify_all();
}

void Resolver::clear()
{
    Lock lock{mutex_};
    // This will unblock waiters by throwing a "broken promise" exception.
    return queue_.clear();
}

AddrInfoFuture Resolver::resolve(const std::string& uri, int type)
{
    Task task{[=]() -> AddrInfoPtr { return parse_endpoint(uri, type); }};
    auto future = task.get_future();

    Lock lock{mutex_};
    if (stop_) {
        throw std::logic_error{"resolver stopped"};
    }
    queue_.push_back(std::move(task));
    lock.unlock();
    cond_.notify_one();
    return future;
}

} // namespace net
} // namespace toolbox
