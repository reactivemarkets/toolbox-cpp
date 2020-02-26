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

#include "RateLimit.hpp"

#include <toolbox/util/String.hpp>

namespace toolbox {
inline namespace net {
using namespace std;

RateLimit parse_rate_limit(const string& s)
{
    const auto [limit, interval] = split_pair(s, '/');
    return RateLimit{ston<size_t>(limit), Seconds{ston<int>(interval)}};
}

istream& operator>>(istream& is, RateLimit& rl)
{
    string s;
    if (is >> s) {
        rl = parse_rate_limit(s);
    }
    return is;
}

ostream& operator<<(ostream& os, RateLimit rl)
{
    return os << rl.limit() << '/' << rl.interval().count();
}

RateWindow::RateWindow(Seconds interval)
: buckets_(interval.count())
{
}

RateWindow::~RateWindow() = default;

// Copy.
RateWindow::RateWindow(const RateWindow&) = default;
RateWindow& RateWindow::operator=(const RateWindow&) = default;

// Move.
RateWindow::RateWindow(RateWindow&&) = default;
RateWindow& RateWindow::operator=(RateWindow&&) = default;

void RateWindow::add(MonoTime time, size_t count) noexcept
{
    const auto t = MonoClock::to_time_t(time);
    auto& bucket = at(t);
    // The fast path and simplest case is where the time falls within the same time bucket as the
    // previous tick.
    if (t == last_time_) {
        count_ += count;
        bucket += count;
        return;
    }
    // Clock is assumed to be monotonic, so if the time is not the same, then it must be greater.
    assert(t > last_time_);
    // If the time has advanced by less than one complete cycle of the buffers.
    if (t - last_time_ < static_cast<time_t>(buckets_.size())) {
        // Advance past any interleaving buckets.
        while (t != ++last_time_) {
            auto& skipped = at(last_time_);
            count_ -= skipped;
            skipped = 0;
        }
        // And then adjust the count on the target bucket.
        count_ = count_ + count - bucket;
    } else {
        // Zero the entire buffer.
        fill(buckets_.begin(), buckets_.end(), 0);
        count_ = count;
    }
    last_time_ = t;
    bucket = count;
}

} // namespace net
} // namespace toolbox
