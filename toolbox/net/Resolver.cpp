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

#include "Resolver.hpp"

#include <toolbox/net/Endpoint.hpp>

namespace toolbox {
inline namespace net {

bool Resolver::run()
{
    return tq_.run([](Task&& t) { t(); });
}

void Resolver::stop()
{
    tq_.stop();
}

void Resolver::clear()
{
    // This will unblock waiters by throwing a "broken promise" exception.
    return tq_.clear();
}

AddrInfoFuture Resolver::resolve(std::string_view uri, int type)
{
    Task task{[=]() -> AddrInfoPtr { return parse_endpoint(uri, type); }};
    auto future = task.get_future();
    tq_.push(std::move(task));
    return future;
}

} // namespace net
} // namespace toolbox
