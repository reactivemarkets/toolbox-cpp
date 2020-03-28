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

#ifndef TOOLBOX_SYS_THREAD_HPP
#define TOOLBOX_SYS_THREAD_HPP

#include <toolbox/Config.h>

#include <string>

namespace toolbox {
inline namespace sys {

/// ThreadConfig holds the thread attributes.
struct ThreadConfig {
    ThreadConfig(std::string name, std::string affinity) noexcept
    : name{std::move(name)}
    , affinity{std::move(affinity)}
    {
    }
    ThreadConfig(std::string name) noexcept
    : name{std::move(name)}
    {
    }
    ThreadConfig() noexcept = default;

    // Copy.
    ThreadConfig(const ThreadConfig&) = default;
    ThreadConfig& operator=(const ThreadConfig&) = default;

    // Move.
    ThreadConfig(ThreadConfig&&) noexcept = default;
    ThreadConfig& operator=(ThreadConfig&&) noexcept = default;

    /// The thread's name.
    std::string name;
    /// The thread's affinity.
    std::string affinity;
};

/// Parse an isolcpus-style set of CPUs.
///
/// The CPU set is either a list: "<cpu>,...,<cpu>", or a range: "<cpu>-<cpu>", or a combination or
/// both: "<cpu>,...,<cpu>-<cpu>", where "<cpu>" begins at 0 and the maximum value is "number of
/// CPUs in system - 1". For example: "0,1,10-11,22-23".
///
/// \param s An isolcpus-style set of CPUs.
/// \return a copy of the CPU set.
TOOLBOX_API cpu_set_t parse_cpu_set(std::string_view s) noexcept;

/// Set attributes for current thread from config.
/// \param config The configuration.
TOOLBOX_API void set_thread_attrs(const ThreadConfig& config);

} // namespace sys
} // namespace toolbox

#endif // TOOLBOX_SYS_THREAD_HPP
