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

#include "VarSub.hpp"

#include <cstdlib> // getenv()

namespace toolbox {
inline namespace util {
using namespace std;

string get_env(const string& name)
{
    const char* const val{getenv(name.c_str())};
    return val ? string{val} : string{};
}

VarSub::~VarSub() = default;

// Copy.
VarSub::VarSub(const VarSub&) = default;
VarSub& VarSub::operator=(const VarSub&) = default;

// Move.
VarSub::VarSub(VarSub&&) = default;
VarSub& VarSub::operator=(VarSub&&) = default;

bool VarSub::substitute(string& s, const size_t i, size_t j, set<string>* outer) const
{
    // Position of last substitution.
    size_t last{0};
    // Names substituted at 'last' position.
    set<string> inner;

    int state{0};
    while (j < s.size()) {
        if (state == '\\') {
            state = 0;
            // Remove backslash.
            s.erase(j - 1, 1);
        } else {
            const auto ch = s[j];
            if (state == '$') {
                state = 0;
                if (ch == '{') {
                    if (j > last) {
                        // Position has advanced.
                        last = j;
                        inner.clear();
                    }
                    // Reverse to '$'.
                    --j;
                    // Descend: search for closing brace and substitute.
                    if (!substitute(s, j, j + 2, &inner)) {
                        return false;
                    }
                    continue;
                }
            }
            switch (ch) {
            case '$':
            case '\\':
                state = ch;
                break;
            case '}':
                // If outer is null then the closing brace was found at the top level. I.e. there is no
                // matching opening brace.
                if (outer) {
                    // Substitute variable.
                    const auto n = j - i;
                    auto name = s.substr(i + 2, n - 2);
                    if (outer->count(name) == 0) {
                        s.replace(i, n + 1, fn_(name));
                        outer->insert(move(name));
                    } else {
                        // Loop detected: this name has already been substituted at this position.
                        s.erase(i, n + 1);
                    }
                    // Ascend: matched closing brace.
                    return true;
                }
                break;
            }
        }
        ++j;
    }
    // Ascend: no closing brace.
    return false;
}

} // namespace util
} // namespace toolbox
