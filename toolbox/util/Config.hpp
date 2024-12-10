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

#ifndef TOOLBOX_UTIL_CONFIG_HPP
#define TOOLBOX_UTIL_CONFIG_HPP

#include <toolbox/util/TypeTraits.hpp>
#include <toolbox/util/Utility.hpp>
#include <toolbox/util/String.hpp>

#include <ranges>
#include <stdexcept>
#include <unordered_map>
#include <map>

namespace toolbox {
inline namespace util {

template <typename FnT>
std::istream& parse_section(std::istream& is, FnT fn, std::string* name = nullptr)
{
    using namespace std::literals::string_literals;

    enum class KeyClassification {SingleValued, MultiValued};
    std::unordered_map<std::string, KeyClassification, string_hash, std::equal_to<>> key_class_map;

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

        bool is_multi = [&line]() {
            auto multi_pos = line.find("+=");
            auto single_pos = line.find('=');
            return multi_pos < single_pos;
        }();

        auto delim = std::string_view{is_multi ? "+=" : "="};

        auto [key, val] = split_pair(line, delim);
        rtrim(key);
        ltrim(val);

        std::optional<KeyClassification> key_class;
        if (auto it = key_class_map.find(key); it != key_class_map.end()) {
            key_class = (*it).second;
        }

        if (key_class) [[unlikely]] {
            if (*key_class == KeyClassification::SingleValued && is_multi) {
                throw std::runtime_error{
                    std::string{key}.append(" is already set as a single-valued key (with '=') "
                                            "and cannot be reassigned with '+='")};
            }
            else if (*key_class == KeyClassification::MultiValued && !is_multi) {
                throw std::runtime_error{
                    std::string{key}.append(" is already set as a multi-valued key (with '+=') "
                                            "and cannot be reassigned with '='")};
            }
        }
        else {
            key_class_map[key] = is_multi ? KeyClassification::MultiValued
                                          : KeyClassification::SingleValued;
        }

        fn(std::move(key), std::move(val));
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
    Config(Config&&) noexcept;
    Config& operator=(Config&&) noexcept;

    /// Throws std::runtime_error if key does not exist.
    const std::string& get(const std::string& key) const;
    const char* get(const std::string& key, const char* dfl) const;
    const char* get(const std::string& key, std::nullptr_t) const
    {
        return get(key, static_cast<const char*>(nullptr));
    }
    /// Throws std::runtime_error if key does not exist.
    template <typename ValueT>
    ValueT get(const std::string& key) const
    {
        const auto it{get_last_value(key)};
        if (it != map_.end()) {
            return transform_value<ValueT>(it->second);
        }
        if (!parent_) {
            throw std::runtime_error{std::string{"missing config key: "} + key};
        }
        return parent_->get<ValueT>(key);
    }
    template <typename ValueT>
    ValueT get(const std::string& key, ValueT dfl) const
    {
        const auto it{get_last_value(key)};
        if (it != map_.end()) {
            return transform_value<ValueT>(it->second);
        }
        return parent_ ? parent_->get<ValueT>(key, dfl) : dfl;
    }

    auto get_multi(const std::string& key) const
    {
        auto [begin, end] = map_.equal_range(key);

        auto fn = [](const auto& kvp) -> const std::string& { return kvp.second; };
        auto rng = std::ranges::subrange(begin, end, map_.count(key)) | std::views::transform(fn);

        if (!rng.empty()) {
            return rng;
        }

        return parent_ ? parent_->get_multi(key) : rng;
    }

    template <typename ValueT>
    auto get_multi(const std::string& key) const {
        return get_multi(key) | std::views::transform(transform_value<ValueT>);
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
    void insert(std::string key, std::string val)
    {
        map_.emplace(std::move(key), std::move(val));
    }

    // existing values for given key are erased, and new values are inserted
    template<typename... ValueTs>
        requires(toolbox::is_string_type_v<ValueTs> && ...)
    void set(const std::string& key, ValueTs ... vals)
    {
        static_assert(sizeof...(ValueTs) > 0, "at least 1 value is required");

        map_.erase(key);
        (insert(key, std::string(std::move(vals))), ...);
    }

    void set_parent(Config& parent) noexcept { parent_ = &parent; }

  private:
    template <typename ValueT>
    static ValueT transform_value(const std::string& v)
    {
        if constexpr (std::is_same_v<ValueT, std::string_view>) {
            // Explicitly allow conversion to std::string_view in this case,
            // because we know that the argument is not a temporary.
            return std::string_view{v};
        } else if constexpr (std::is_enum_v<ValueT>) {
            return ValueT{from_string<std::underlying_type_t<ValueT>>(v)};
        } else {
            return from_string<ValueT>(v);
        };
    }

    auto get_last_value(const std::string& key) const
        -> std::multimap<std::string, std::string>::const_iterator
    {
        auto [first, second] = map_.equal_range(key);
        return (first == second) ? map_.end() : std::prev(second);
    }

    std::istream& read_section(std::istream& is, std::string* next);
    std::multimap<std::string, std::string> map_;
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
    MultiConfig(MultiConfig&&) noexcept;
    MultiConfig& operator=(MultiConfig&&) noexcept;

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
