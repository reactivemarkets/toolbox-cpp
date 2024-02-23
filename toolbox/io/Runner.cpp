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
void run_reactor(Reactor& r, long busy_cycles, ThreadConfig config, const std::atomic<bool>& stop)
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

HistogramPtr make_time_histogram()
{
    // Record microseconds with 3sf and max expected value of one second.
    return HistogramPtr{new Histogram{1, 1'000'000, 3}};
}

HistogramPtr make_work_histogram()
{
    // Histogram is 100% accurate to 256, covering Reactor::MaxEvents of 128 work items.
    return HistogramPtr{new Histogram{1, 1000, 2}};
}

void run_metrics_reactor(Reactor& r, long busy_cycles, ThreadConfig config,
                         const std::atomic<bool>& stop, MetricCallbackFunction metric_cb,
                         LoopCallbackFunction loop_cb)
{
    constexpr std::chrono::seconds MetricInterval = 60s;

    // ==== KEEP IN SYNC WITH CHANGES IN run_reactor ====
    sig_block_all();
    try {
        set_thread_attrs(config);
        TOOLBOX_NOTICE << "started " << config.name << " thread";

        HistogramPtr time_hist = make_time_histogram();
        // 128 possible buffer slots in poll + high and low priority timers
        HistogramPtr work_hist = make_work_histogram();

        long i{0};
        auto metric_time = MonoClock::now() + MetricInterval;
        while (!stop.load(std::memory_order_acquire)) {
            // Busy-wait for "busy cycles" after work was done.
            auto work = r.poll(CyclTime::now(), i++ < busy_cycles ? 0s : NoTimeout);
            const auto now = CyclTime::current();
            if (work > 0) {
                // Don't skew distribution with a lot of zero work.
                const auto elapsed_us = std::chrono::duration_cast<std::chrono::microseconds>(
                    MonoClock::now() - now.mono_time());
                time_hist->record_value(elapsed_us.count());
                work_hist->record_value(work);
                loop_cb(now);
                // Reset counter when work has been done.
                i = 0;
            }
            if (now.mono_time() >= metric_time) {
                // Metric reporting.
                metric_time = now.mono_time() + MetricInterval;
                metric_cb(now, std::move(time_hist), std::move(work_hist));
                time_hist = make_time_histogram();
                work_hist = make_work_histogram();
            }
        }
    } catch (const std::exception& e) {
        TOOLBOX_CRIT << "exception on " << config.name << " thread: " << e.what();
        kill(getpid(), SIGTERM);
    }
    TOOLBOX_NOTICE << "stopping " << config.name << " thread";
}

} // namespace

ReactorRunner::ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config)
: reactor_{r}
, thread_{run_reactor, std::ref(r), busy_cycles, config, std::cref(stop_)}
{
}

ReactorRunner::ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config,
                             MetricCallbackFunction metric_cb)
: ReactorRunner(r, busy_cycles, config, metric_cb, [](CyclTime) {})
{
}

ReactorRunner::ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config,
                             MetricCallbackFunction metric_cb, LoopCallbackFunction loop_cb)
: reactor_{r}
, thread_{run_metrics_reactor, std::ref(r), busy_cycles, config, std::cref(stop_),
          metric_cb, loop_cb}
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
