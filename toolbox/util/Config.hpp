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

#ifndef TOOLBOX_UTIL_CONFIG_HPP
#define TOOLBOX_UTIL_CONFIG_HPP

#include <toolbox/util/String.hpp>
#include <toolbox/util/TypeTraits.hpp>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#include <boost/container/flat_map.hpp>
#pragma GCC diagnostic pop

#include <map>
#include <string>

namespace toolbox {
inline namespace util {

template <typename FnT>
std::istream& parse_section(std::istream& is, FnT fn, std::string* name = nullptr)
{
    std::string line;
    while (std::getline(is, line)) {

        trim(line);

        // Ignore empty lines and comments.

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line.front() == '[' && line.back() == ']') {
            if (name) {
                name->assign(line, 1, line.size() - 2);
                trim(*name);
            }
            return is;
        }

        auto [key, val] = split_pair(line, '=');
        rtrim(key);
        ltrim(val);

        fn(key, val);
    }
    if (name) {
        name->clear();
    }
    return is;
}

/// Simple config reader with environment variable substitution.
class TOOLBOX_API Config {
  public:
    Config();
    ~Config();

    // Copy.
    Config(const Config&);
    Config& operator=(const Config&);

    // Move.
    Config(Config&&);
    Config& operator=(Config&&);

    /// Throws std::runtime_error if key does not exist.
    const std::string& get(const std::string& key) const;
    const char* get(const std::string& key, const char* dfl) const noexcept;
    const char* get(const std::string& key, std::nullptr_t) const noexcept
    {
        return get(key, static_cast<const char*>(nullptr));
    }
    /// Throws std::runtime_error if key does not exist.
    template <typename ValueT>
    ValueT get(const std::string& key) const
    {
        auto it = map_.find(key);
        if (it != map_.end()) {
            if constexpr (std::is_same_v<ValueT, std::string_view>) {
                // Explicitly allow conversion to std::string_view in this case, because we know
                // that the argument is not a temporary.
                return from_string<ValueT>(std::string_view{it->second});
            } else if constexpr (std::is_enum_v<ValueT>) {
                return ValueT{from_string<std::underlying_type_t<ValueT>>(it->second)};
            } else {
                return from_string<ValueT>(it->second);
            }
        }
        if (!parent_) {
            throw std::runtime_error{"missing config key: "s + key};
        }
        return parent_->get<ValueT>(key);
    }
    template <typename ValueT>
    ValueT get(const std::string& key, ValueT dfl) const noexcept
    {
        auto it = map_.find(key);
        if (it != map_.end()) {
            if constexpr (std::is_same_v<ValueT, std::string_view>) {
                // Explicitly allow conversion to std::string_view in this case, because we know
                // that the argument is not a temporary.
                return from_string<ValueT>(std::string_view{it->second});
            } else if constexpr (std::is_enum_v<ValueT>) {
                return ValueT{from_string<std::underlying_type_t<ValueT>>(it->second)};
            } else {
                return from_string<ValueT>(it->second);
            }
        }
        return parent_ ? parent_->get<ValueT>(key, dfl) : dfl;
    }
    std::size_t size() const noexcept { return map_.size(); }
    void clear() noexcept { map_.clear(); }
    std::istream& read_section(std::istream& is) { return read_section(is, nullptr); }
    std::istream& read_section(std::istream&& is) { return read_section(is); }
    std::istream& read_section(std::istream& is, std::string& next)
    {
        return read_section(is, &next);
    }
    std::istream& read_section(std::istream&& is, std::string& next)
    {
        return read_section(is, next);
    }
    void set(std::string key, std::string val) { map_.emplace(std::move(key), std::move(val)); }
    void set_parent(Config& parent) noexcept { parent_ = &parent; }

  private:
    std::istream& read_section(std::istream& is, std::string* next);
    boost::container::flat_map<std::string, std::string> map_;
    Config* parent_{nullptr};
};

class TOOLBOX_API MultiConfig {
  public:
    MultiConfig();
    ~MultiConfig();

    // Copy.
    MultiConfig(const MultiConfig&) = delete;
    MultiConfig& operator=(const MultiConfig&) = delete;

    // Move.
    MultiConfig(MultiConfig&&);
    MultiConfig& operator=(MultiConfig&&);

    void clear() noexcept;
    void read(std::istream& is);
    void read(std::istream&& is) { read(is); }

    const Config& root() const noexcept { return root_; }
    const Config& section(const std::string& name) const noexcept
    {
        auto it = map_.find(name);
        return it != map_.end() ? it->second : root_;
    }
    const Config& section(std::string_view name) const noexcept
    {
        return section(std::string{name});
    }

  private:
    Config root_;
    // Map of named sections.
    std::map<std::string, Config> map_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_CONFIG_HPP
