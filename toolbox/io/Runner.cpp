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

#include "Runner.hpp"

#include "Reactor.hpp"

#include <toolbox/sys/Log.hpp>
#include <toolbox/sys/Signal.hpp>

namespace toolbox {
inline namespace io {
namespace {
void run_reactor(Reactor& r, long busy_cycles, ThreadConfig config, const std::atomic<bool>& stop,
                 [[maybe_unused]] MetricCallbackFunction metric_callback)
{
    // ==== KEEP IN SYNC WITH CHANGES IN run_metrics_reactor ====
    sig_block_all();
    try {
        set_thread_attrs(config);
        TOOLBOX_NOTICE << "started " << config.name << " thread";
        long i{0};
        while (!stop.load(std::memory_order_acquire)) {
            // Busy-wait for "busy cycles" after work was done.
            if (r.poll(CyclTime::now(), i++ < busy_cycles ? 0s : NoTimeout) > 0) {
                // Reset counter when work has been done.
                i = 0;
            }
        }
    } catch (const std::exception& e) {
        TOOLBOX_CRIT << "exception on " << config.name << " thread: " << e.what();
        kill(getpid(), SIGTERM);
    }
    TOOLBOX_NOTICE << "stopping " << config.name << " thread";
}

static const std::chrono::seconds metric_interval = 60s;
void run_metrics_reactor(Reactor& r, long busy_cycles, ThreadConfig config,
                         const std::atomic<bool>& stop, MetricCallbackFunction metric_callback)
{
    // ==== KEEP IN SYNC WITH CHANGES IN run_reactor ====
    sig_block_all();
    try {
        // 128 possible buffer slots in poll + high and low priority timers
        Histogram hist{1, 1000, 2}; // 100% accutate to 256
        set_thread_attrs(config);
        TOOLBOX_NOTICE << "started " << config.name << " thread";
        long i{0};
        auto metric_time = WallClock::now() + metric_interval;
        while (!stop.load(std::memory_order_acquire)) {
            // Busy-wait for "busy cycles" after work was done.
            const CyclTime& now = CyclTime::now();
            auto work = r.poll(now, i++ < busy_cycles ? 0s : NoTimeout);
            if (work > 0) {
                // Don't skew distribution with a lot of zero work.
                hist.record_value(work);
                // Reset counter when work has been done.
                i = 0;
            }
            if (now.wall_time() >= metric_time) {
                // Metric reporting
                metric_time = now.wall_time() + metric_interval;
                metric_callback(now, hist);
                hist.reset();
            }
        }
    } catch (const std::exception& e) {
        TOOLBOX_CRIT << "exception on " << config.name << " thread: " << e.what();
        kill(getpid(), SIGTERM);
    }
    TOOLBOX_NOTICE << "stopping " << config.name << " thread";
}

} // namespace

ReactorRunner::ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config, MetricCallbackFunction metric_callback)
: reactor_{r}
, thread_{metric_callback == nullptr ? run_reactor : run_metrics_reactor,
          std::ref(r), busy_cycles, config, std::cref(stop_), metric_callback}
{
}

ReactorRunner::~ReactorRunner()
{
    stop_.store(true, std::memory_order_release);
    reactor_.wakeup();
    thread_.join();
}

} // namespace io
} // namespace toolbox
