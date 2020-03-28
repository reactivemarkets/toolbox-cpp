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

#include "Runner.hpp"

#include "Resolver.hpp"

#include <toolbox/io/Reactor.hpp>

#include <toolbox/sys/Log.hpp>
#include <toolbox/sys/Signal.hpp>

namespace toolbox {
inline namespace net {
namespace {
void run_resolver(Resolver& r, ThreadConfig config)
{
    sig_block_all();
    try {
        set_thread_attrs(config);
        TOOLBOX_NOTICE << "started " << config.name << " thread";
        // The run() function returns -1 when resolver is closed.
        while (r.run() >= 0)
            ;
    } catch (const std::exception& e) {
        TOOLBOX_CRIT << "exception: " << e.what();
        kill(getpid(), SIGTERM);
    }
    TOOLBOX_NOTICE << "stopping " << config.name << " thread";
}
} // namespace

ResolverRunner::ResolverRunner(Resolver& r, ThreadConfig config)
: resolver_{r}
, thread_{run_resolver, std::ref(r), config}
{
}

ResolverRunner::~ResolverRunner()
{
    resolver_.stop();
    thread_.join();
}

} // namespace net
} // namespace toolbox
