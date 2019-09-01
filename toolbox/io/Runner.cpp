// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
// Copyright (C) 2019 Reactive Markets Limited
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
void run_reactor(Reactor& r, ThreadConfig config, const std::atomic<bool>& stop)
{
    sig_block_all();
    pthread_setname_np(pthread_self(), config.name.c_str());
    TOOLBOX_NOTICE << "started " << config.name << " thread";
    try {
        while (!stop.load(std::memory_order_acquire)) {
            r.poll(CyclTime::now());
        }
    } catch (const std::exception& e) {
        TOOLBOX_CRIT << "exception: " << e.what();
        kill(getpid(), SIGTERM);
    }
    TOOLBOX_NOTICE << "stopping " << config.name << " thread";
}
} // namespace

ReactorRunner::ReactorRunner(Reactor& r, ThreadConfig config)
: reactor_(r)
, thread_{run_reactor, std::ref(r), config, std::cref(stop_)}
{
}

ReactorRunner::~ReactorRunner()
{
    stop_.store(true, std::memory_order_release);
    reactor_.notify();
    thread_.join();
}

} // namespace io
} // namespace toolbox
