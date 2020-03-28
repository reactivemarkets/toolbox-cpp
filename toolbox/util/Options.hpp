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

#ifndef TOOLBOX_UTIL_OPTIONS_HPP
#define TOOLBOX_UTIL_OPTIONS_HPP

/// \file Options.hpp
///
/// \brief Command-line options parser.
///
/// This file was originally authored by Rodrigo Fernandes as part of the Swirly Cloud project.
///
/// \author Rodrigo Fernandes

#include <toolbox/util/RefCount.hpp>
#include <toolbox/util/String.hpp>

#include <functional>
#include <map>
#include <variant>
#include <vector>

namespace toolbox {
inline namespace util {

class Options;

class NoOp {
};

class Help {
};

template <typename DerivedT>
class Presence {
  public:
    enum Type { Optional, Required };

    DerivedT& required()
    {
        presence_ = Required;
        return *static_cast<DerivedT*>(this);
    }

    Type presence() const noexcept { return presence_; }

  private:
    Type presence_{Optional};
};

class Value : public Presence<Value> {

  public:
    template <typename VarT>
    explicit Value(VarT& var)
    : func_{[&var](std::string_view arg) { var = from_string<VarT>(arg); }}
    {
    }

    template <typename VarT, typename AllocT>
    explicit Value(std::vector<VarT, AllocT>& var)
    : func_{[&var](std::string_view arg) { var.push_back(from_string<VarT>(arg)); }}
    {
    }

    template <typename ValueT>
    Value& default_value(const ValueT& value)
    {
        if constexpr (is_string<ValueT>::value) {
            func_(value);
        } else {
            func_(std::to_string(value));
        }
        return *this;
    }
    Value& multitoken()
    {
        multitoken_ = true;
        return *this;
    }

  private:
    friend class Options;

    void run(std::string_view arg)
    {
        func_(arg);
        set_ = true;
    }

    bool set() const noexcept { return set_; }

    std::function<void(std::string_view)> func_;
    bool set_{false};
    bool multitoken_{false};
};

class Switch : public Presence<Switch> {
  public:
    Switch(bool& flag)
    : flag_{flag}
    {
    }

  private:
    friend class Options;

    void run() { flag_ = true; }

    bool& flag_;
};

class TOOLBOX_API Options {

    struct OptionCompare {
        using is_transparent = void;
        bool operator()(const std::string& lhs, const std::string& rhs) const noexcept
        {
            return lhs < rhs;
        }
        bool operator()(const std::string& lhs, std::string_view rhs) const noexcept
        {
            return lhs < rhs;
        }
        bool operator()(std::string_view lhs, const std::string& rhs) const noexcept
        {
            return lhs < rhs;
        }
    };

    using Data = std::variant<Value, Help, NoOp, Switch>;

    struct OptionData : RefCount<OptionData, ThreadUnsafePolicy> {
        template <typename DataT>
        OptionData(char short_opt, const std::string& long_opt, DataT data, std::string description)
        : long_opt{long_opt}
        , data{std::move(data)}
        , description{std::move(description)}
        {
            if (short_opt) {
                this->short_opt.push_back(short_opt);
            }
        }
        const std::string& opt() const noexcept { return long_opt.empty() ? short_opt : long_opt; }

        std::string short_opt, long_opt;
        Data data;
        std::string description;
    };

    using OptionDataPtr = boost::intrusive_ptr<OptionData>;

  public:
    explicit Options(std::string description = "");

    template <typename DataT>
    Options& operator()(const std::string& long_opt, DataT&& option_data,
                        std::string description = "")
    {
        this->operator()(0, long_opt, std::forward<DataT>(option_data), std::move(description));
        return *this;
    }

    template <typename DataT>
    Options& operator()(char short_opt, const std::string& long_opt, DataT&& data,
                        std::string description = "")
    {
        auto opt_data = make_intrusive<OptionData>(short_opt, long_opt, std::forward<DataT>(data),
                                                   std::move(description));
        help_.push_back(opt_data);
        if (short_opt) {
            const bool inserted = opts_.emplace(opt_data->short_opt, opt_data).second;
            if (!inserted) {
                throw std::runtime_error{"attempting to register duplicate option "
                                         + opt_data->short_opt};
            }
        }
        if (!long_opt.empty()) {
            const bool inserted = opts_.emplace(long_opt, opt_data).second;
            if (!inserted) {
                throw std::runtime_error{"attempting to register duplicate option " + long_opt};
            }
        }
        return *this;
    }

    template <typename DataT>
    Options& operator()(DataT data, std::string description = "")
    {
        positional_.push_back(std::move(data));
        return *this;
    }

    template <typename DataT>
    Options& operator()(char short_opt, DataT&& option_data, std::string description = "")
    {
        this->operator()(short_opt, "", std::forward<DataT>(option_data), std::move(description));
        return *this;
    }

    bool operator[](const std::string& long_opt) const noexcept;
    bool operator[](char short_opt) const noexcept;
    void parse(int argc, const char* const argv[]);

  private:
    friend std::ostream& operator<<(std::ostream& out, const Options& options);

    std::string description_;
    using HelpVec = std::vector<OptionDataPtr>;
    HelpVec help_;
    using OptsMap = std::map<std::string, OptionDataPtr, OptionCompare>;
    OptsMap opts_;
    using DataVec = std::vector<Data>;
    DataVec positional_;
};

TOOLBOX_API std::ostream& operator<<(std::ostream& out, const Options& options);

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_OPTIONS_HPP
