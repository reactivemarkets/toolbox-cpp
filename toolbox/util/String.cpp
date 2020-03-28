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

#include "String.hpp"

namespace toolbox {
inline namespace util {
using namespace std;

namespace {
constexpr char Space[] = " \t\n\v\f\r";
} // namespace

void ltrim(string_view& s) noexcept
{
    const auto pos = s.find_first_not_of(Space);
    s.remove_prefix(pos != string_view::npos ? pos : s.size());
}

void ltrim(string& s) noexcept
{
    const auto pos = s.find_first_not_of(Space);
    s.erase(0, pos != string_view::npos ? pos : s.size());
}

void rtrim(string_view& s) noexcept
{
    const auto pos = s.find_last_not_of(Space);
    s.remove_suffix(s.size() - (pos != string_view::npos ? pos + 1 : 0));
}

void rtrim(string& s) noexcept
{
    const auto pos = s.find_last_not_of(Space);
    s.erase(pos != string_view::npos ? pos + 1 : 0);
}

pair<string_view, string_view> split_pair(string_view s, char delim) noexcept
{
    const auto pos = s.find_first_of(delim);
    string_view key, val;
    if (pos == string_view::npos) {
        key = s;
    } else {
        key = s.substr(0, pos);
        val = s.substr(pos + 1);
    }
    return {key, val};
}

pair<string, string> split_pair(const string& s, char delim)
{
    const auto pos = s.find_first_of(delim);
    string key, val;
    if (pos == string::npos) {
        key = s;
    } else {
        key = s.substr(0, pos);
        val = s.substr(pos + 1);
    }
    return {key, val};
}

} // namespace util
} // namespace toolbox
