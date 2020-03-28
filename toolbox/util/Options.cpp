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

#include "Options.hpp"

#include <toolbox/util/Argv.hpp>
#include <toolbox/util/Variant.hpp>

#include <iomanip>
#include <iostream>

namespace toolbox {
inline namespace util {
using namespace std;

Options::Options(string description)
: description_{move(description)}
{
}

bool Options::operator[](const string& long_opt) const noexcept
{
    return opts_.count(long_opt);
}

bool Options::operator[](char short_opt) const noexcept
{
    return opts_.count(std::string_view{&short_opt, 1});
}

void Options::parse(int argc, const char* const argv[])
{
    size_t positional_idx{0};
    ArgvLexer lex{argc - 1, argv + 1};
    while (!lex.empty()) {
        const auto opt = lex.opt();

        bool positional{false};
        OptsMap::iterator it{opts_.end()};

        if (!opt.empty()) {
            it = opts_.find(opt);
        } else {
            if (positional_idx >= positional_.size()) {
                throw runtime_error{"unexpected argument: " + string{lex.arg()}};
            }
            positional = true;
        }

        if (!positional && it == opts_.end()) {
            throw runtime_error{"unknown option: " + string{opt}};
        }

        auto& data = positional ? positional_[positional_idx++] : it->second->data;

        visit(overloaded{[&](const Help& arg) {
                             lex.pop_switch();
                             cout << *this << "\n";
                             exit(0);
                         },
                         [&](Switch& arg) {
                             lex.pop_switch();
                             arg.run();
                         },
                         [&](Value& arg) {
                             const auto val = lex.pop_value();
                             if (arg.set_ && !arg.multitoken_) {
                                 throw runtime_error{"duplicate option: " + string{opt}};
                             }
                             arg.run(val);
                         },
                         [&](const NoOp& arg) { lex.pop(); }},
              data);
    }

    // Ensure all required parameters have been set.
    // FIXME: but we're not checking required positional arguments?
    for (const auto& data : opts_) {
        visit(overloaded{[](const auto& def) {},
                         [&](const Value& arg) {
                             if (arg.presence() == Value::Required && !arg.set()) {
                                 throw runtime_error{"missing required option: "
                                                     + data.second->opt()};
                             }
                         }},
              data.second->data);
    }
    for (const auto& data : positional_) {
        visit(overloaded{[](const auto& def) {},
                         [&](const Value& arg) {
                             if (arg.presence() == Value::Required && !arg.set()) {
                                 throw runtime_error{"missing positional argument"};
                             }
                         }},
              data);
    }
}

ostream& operator<<(ostream& out, const Options& options)
{
    out << "Usage: " << options.description_ << "\nOptions:\n";

    for (const auto& opt : options.help_) {
        unsigned max_width{15};
        // value find
        out << "  ";
        if (!opt->short_opt.empty()) {
            max_width -= 2;
            out << '-' << opt->short_opt;
        }
        if (!opt->long_opt.empty()) {
            if (!opt->short_opt.empty()) {
                max_width -= 2;
                out << ", ";
            }
            max_width -= 2 + opt->long_opt.size();
            out << "--" << opt->long_opt;
        }
        out << setw(max_width) << ' ' << opt->description << "\n";
    }
    return out;
}

} // namespace util
} // namespace toolbox
