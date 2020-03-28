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

#include "Argv.hpp"

namespace toolbox {
inline namespace util {
using namespace std;

ArgvLexer::~ArgvLexer() = default;

void ArgvLexer::pop_switch()
{
    ++it_;
    if (val_) {
        throw runtime_error{"unexpected value for option: " + string{opt_}};
    }
    next();
}

string_view ArgvLexer::pop_value()
{
    ++it_;
    string_view val;
    if (val_) {
        val = *val_;
    } else {
        if (empty()) {
            throw runtime_error{"missing value for option: " + string{opt_}};
        }
        val = *it_++;
    }
    next();
    return val;
}

void ArgvLexer::next() noexcept
{
    if (empty()) {
        opt_ = {};
        val_.reset();
        return;
    }
    const string_view arg{*it_};
    if (!(arg.size() > 1 && arg[0] == '-')) {
        // Positional argument.
        opt_ = {};
        val_ = arg;
        return;
    }

    if (arg[1] == '-') {
        if (arg.size() == 2) {
            // Treat double dash as positional argument.
            opt_ = {};
            val_ = arg;
            return;
        }
        // Long option.
        const auto pos = arg.find_first_of('=', 2);
        opt_ = arg.substr(2, pos - 2);
        if (pos != string_view::npos) {
            val_ = arg.substr(pos + 1);
        } else {
            val_.reset();
        }
    } else {
        // Short option.
        opt_ = arg.substr(1, 1);
        if (arg.size() > 2) {
            val_ = arg.substr(2);
        } else {
            val_.reset();
        }
    }
}

} // namespace util
} // namespace toolbox
