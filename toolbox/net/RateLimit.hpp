// The Reactive C++ Toolbox.
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

#ifndef TOOLBOX_NET_RATELIMIT_HPP
#define TOOLBOX_NET_RATELIMIT_HPP

#include <toolbox/sys/Time.hpp>

#include <boost/container/small_vector.hpp>

namespace toolbox {
inline namespace net {

/// RateLimit represents a rate limit spanning an interval of one or more seconds.
/// Rate limiters control the rate of requests sent or received over the network.
class TOOLBOX_API RateLimit {
  public:
    constexpr RateLimit(std::size_t limit, Seconds interval) noexcept
    : limit_{limit}
    , interval_{interval}
    {
    }
    ~RateLimit() = default;

    // Copy.
    constexpr RateLimit(const RateLimit&) noexcept = default;
    constexpr RateLimit& operator=(const RateLimit&) noexcept = default;

    // Move.
    constexpr RateLimit(RateLimit&&) noexcept = default;
    constexpr RateLimit& operator=(RateLimit&&) noexcept = default;

    constexpr auto limit() const noexcept { return limit_; }
    constexpr auto interval() const noexcept { return interval_; }

  private:
    std::size_t limit_;
    Seconds interval_;
};

TOOLBOX_API RateLimit parse_rate_limit(const std::string& s);
TOOLBOX_API std::istream& operator>>(std::istream& is, RateLimit& rl);
TOOLBOX_API std::ostream& operator<<(std::ostream& os, RateLimit rl);

/// RateWindow maintains a sliding window of second time buckets for the specified interval.
class TOOLBOX_API RateWindow {
  public:
    explicit RateWindow(Seconds interval);
    ~RateWindow();

    // Copy.
    RateWindow(const RateWindow&);
    RateWindow& operator=(const RateWindow&);

    // Move.
    RateWindow(RateWindow&&);
    RateWindow& operator=(RateWindow&&);

    std::size_t count() const noexcept { return count_; }
    /// Add count to time bucket.
    /// This function assumes a monotonically increasing clock.
    void add(MonoTime time, std::size_t count = 1) noexcept;

  private:
    std::size_t& at(std::time_t t) noexcept { return buckets_[t % buckets_.size()]; }
    std::size_t count_{};
    std::time_t last_time_{};
    boost::container::small_vector<std::size_t, 10> buckets_;
};
} // namespace net

inline namespace util {
template <>
struct TypeTraits<net::RateLimit> {
    static auto from_string(std::string_view sv) { return net::parse_rate_limit(std::string{sv}); }
    static auto from_string(const std::string& s) { return net::parse_rate_limit(s); }
};
} // namespace util
} // namespace toolbox

#endif // TOOLBOX_NET_RATELIMIT_HPP
