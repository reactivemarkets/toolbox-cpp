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

#include "Config.hpp"

#include <toolbox/util/VarSub.hpp>

namespace toolbox {
inline namespace util {
using namespace std;

Config::Config() = default;
Config::~Config() = default;

// Copy.
Config::Config(const Config&) = default;
Config& Config::operator=(const Config&) = default;

// Move.
Config::Config(Config&&) = default;
Config& Config::operator=(Config&&) = default;

const std::string& Config::get(const std::string& key) const
{
    auto it = map_.find(key);
    if (it != map_.end()) {
        return it->second;
    }
    if (!parent_) {
        throw runtime_error{"missing config key: "s + key};
    }
    return parent_->get(key);
}

const char* Config::get(const std::string& key, const char* dfl) const noexcept
{
    auto it = map_.find(key);
    if (it != map_.end()) {
        return it->second.c_str();
    }
    return parent_ ? parent_->get(key, dfl) : dfl;
}

istream& Config::read_section(istream& is, string* next)
{
    VarSub var_sub;
    return parse_section(
        is,
        [this, &var_sub](const auto& key, string val) {
            var_sub(val);
            map_.emplace(key, move(val));
        },
        next);
}

MultiConfig::MultiConfig() = default;
MultiConfig::~MultiConfig() = default;

MultiConfig::MultiConfig(MultiConfig&&) = default;
MultiConfig& MultiConfig::operator=(MultiConfig&&) = default;

void MultiConfig::clear() noexcept
{
    map_.clear();
    root_.clear();
}

void MultiConfig::read(istream& is)
{
    string next;
    if (root_.read_section(is, next).eof()) {
        return;
    }
    do {
        const string name{next};
        Config config;
        config.set_parent(root_);
        config.read_section(is, next);
        map_.emplace(name, move(config));

    } while (!is.eof());
}

} // namespace util
} // namespace toolbox
