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

#ifndef TOOLBOX_NET_RESOLVER_HPP
#define TOOLBOX_NET_RESOLVER_HPP

#include <toolbox/net/Socket.hpp>
#include <toolbox/sys/Time.hpp>

#include <cassert>
#include <deque>
#include <future>

namespace toolbox {
inline namespace net {

using AddrInfoFuture = std::future<AddrInfoPtr>;

/// The Resolver is designed to resolve socket URIs to address endpoints on a background thread,
/// which may include a DNS lookup depending on the URI.
class TOOLBOX_API Resolver {
    using Lock = std::unique_lock<std::mutex>;
    using Task = std::packaged_task<AddrInfoPtr()>;

  public:
    Resolver() = default;
    ~Resolver() = default;

    // Copy.
    Resolver(const Resolver& rhs) = delete;
    Resolver& operator=(const Resolver& rhs) = delete;

    // Move.
    Resolver(Resolver&&) = delete;
    Resolver& operator=(Resolver&&) = delete;

    /// Wait for and execute a batch of enqueued tasks.
    /// The timeout value must be small enough not to overflow when added to the current time.
    ///
    /// \return the number of tasks executed or -1 if the Resolver was closed.
    int run(Duration timeout = Seconds{60});

    /// Interrupt and exit any inprogress call to run().
    void stop();

    /// Clear task queue. Any pending tasks will be cancelled, resulting in a broken promise.
    void clear();

    /// Schedule a URI socket name resolution.
    AddrInfoFuture resolve(const std::string& uri, int type);

  private:
    mutable std::mutex mutex_;
    std::condition_variable cond_;
    std::deque<Task> queue_;
    bool stop_{false};
};

/// Wait for future and convert result to endpoint.
template <typename EndpointT>
EndpointT get_endpoint(AddrInfoFuture& future)
{
    const auto ai = future.get();
    assert(!future.valid());
    return {ai->ai_addr, ai->ai_addrlen, ai->ai_protocol};
}

/// Returns true if future is ready.
template <typename ResultT>
bool is_ready(std::future<ResultT>& future)
{
    switch (future.wait_for(Seconds{0})) {
    case std::future_status::ready:
        return true;
    case std::future_status::deferred:
    case std::future_status::timeout:
        break;
    }
    return false;
}

} // namespace net
} // namespace toolbox

#endif // TOOLBOX_NET_RESOLVER_HPP
