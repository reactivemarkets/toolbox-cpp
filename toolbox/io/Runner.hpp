// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2022 Reactive Markets Limited
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

#ifndef TOOLBOX_IO_RUNNER_HPP
#define TOOLBOX_IO_RUNNER_HPP

#include <toolbox/hdr/Histogram.hpp>
#include <toolbox/sys/Thread.hpp>
#include <toolbox/sys/Time.hpp>

#include <functional>
#include <thread>

namespace toolbox {
inline namespace io {
class Reactor;

struct MetricContext {
    std::string app_name;
    std::string metric_type;
    std::string metric_id;
};

using HistogramPtr = std::unique_ptr<Histogram>;

/// MetricCallbackFunction implementer is responsible for deleting the Histogram.
using MetricCallbackFunction
    = std::function<void(CyclTime, const MetricContext& ctx, HistogramPtr&&)>;

class TOOLBOX_API ReactorRunner {
  public:
    /// Constructs a ReactorRunner instance.
    ///
    /// When work is processed by the Reactor during a call to Reactor::poll, the next 'n' calls to
    /// Reactor::poll, where 'n' is \a busy_cycles, will not cause the thread to block or yield if
    /// no further work is available. This feature can be useful for keeping the reactor warm and
    /// responsive for short periods of time immediately after work is processed.
    ///
    /// \param r The reactor.
    /// \param busy_cycles The number of busy cycles after doing work.
    /// \param config The thread configuration.
    ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config);

    /// Constructs a ReactorRunner instance.
    ///
    /// When work is processed by the Reactor during a call to Reactor::poll, the next 'n' calls to
    /// Reactor::poll, where 'n' is \a busy_cycles, will not cause the thread to block or yield if
    /// no further work is available. This feature can be useful for keeping the reactor warm and
    /// responsive for short periods of time immediately after work is processed.
    ///
    /// \param r The reactor.
    /// \param busy_cycles The number of busy cycles after doing work.
    /// \param config The thread configuration.
    /// \param metric_ctx Metric context. Accessed from background thread so must be const/immutable.
    /// \param metric_cb Metric callback function.
    ReactorRunner(Reactor& r, long busy_cycles, ThreadConfig config,
                  const MetricContext& metric_ctx, MetricCallbackFunction metric_cb);
    ~ReactorRunner();

    // Copy.
    ReactorRunner(const ReactorRunner&) = delete;
    ReactorRunner& operator=(const ReactorRunner&) = delete;

    // Move.
    ReactorRunner(ReactorRunner&&) = delete;
    ReactorRunner& operator=(ReactorRunner&&) = delete;

  private:
    Reactor& reactor_;
    std::atomic<bool> stop_{false};
    std::thread thread_;
};

} // namespace io
} // namespace toolbox

#endif // TOOLBOX_IO_RUNNER_HPP
