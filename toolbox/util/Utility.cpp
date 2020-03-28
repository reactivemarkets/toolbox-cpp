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

#include "Utility.hpp"

#include <boost/spirit/home/qi/numeric/real.hpp>
#include <boost/spirit/home/qi/parse.hpp>

namespace toolbox {
inline namespace util {
using namespace std;

int dec_digits(int64_t i) noexcept
{
    return i < 10000000000 ? i < 100000
            ? i < 100 ? i < 10 ? 1 : 2 : i < 1000 ? 3 : i < 10000 ? 4 : 5
            : i < 10000000 ? i < 1000000 ? 6 : 7 : i < 100000000 ? 8 : i < 1000000000 ? 9 : 10
                           : i < 1000000000000000
            ? i < 1000000000000 ? i < 100000000000 ? 11 : 12
                                : i < 10000000000000 ? 13 : i < 100000000000000 ? 14 : 15
            : i < 100000000000000000 ? i < 10000000000000000 ? 16 : 17
                                     : i < 1000000000000000000 ? 18 : 19;
}

bool stob(string_view sv, bool dfl) noexcept
{
    bool val{dfl};
    switch (sv.size()) {
    case 1:
        switch (sv[0]) {
        case '0':
        case 'F':
        case 'N':
        case 'f':
        case 'n':
            val = false;
            break;
        case '1':
        case 'T':
        case 'Y':
        case 't':
        case 'y':
            val = true;
            break;
        }
        break;
    case 2:
        if ((sv[0] == 'N' || sv[0] == 'n') //
            && (sv[1] == 'O' || sv[1] == 'o')) {
            val = false;
        } else if ((sv[0] == 'O' || sv[0] == 'o') //
                   && (sv[1] == 'N' || sv[1] == 'n')) {
            val = true;
        }
        break;
    case 3:
        if ((sv[0] == 'O' || sv[0] == 'o')    //
            && (sv[1] == 'F' || sv[1] == 'f') //
            && (sv[2] == 'F' || sv[2] == 'f')) {
            val = false;
        } else if ((sv[0] == 'Y' || sv[0] == 'y')    //
                   && (sv[1] == 'E' || sv[1] == 'e') //
                   && (sv[2] == 'S' || sv[2] == 's')) {
            val = true;
        }
        break;
    case 4:
        if ((sv[0] == 'T' || sv[0] == 't')    //
            && (sv[1] == 'R' || sv[1] == 'r') //
            && (sv[2] == 'U' || sv[2] == 'u') //
            && (sv[3] == 'E' || sv[3] == 'e')) {
            val = true;
        }
        break;
    case 5:
        if ((sv[0] == 'F' || sv[0] == 'f')    //
            && (sv[1] == 'A' || sv[1] == 'a') //
            && (sv[2] == 'L' || sv[2] == 'l') //
            && (sv[3] == 'S' || sv[3] == 's') //
            && (sv[4] == 'E' || sv[4] == 'e')) {
            val = false;
        }
        break;
    }
    return val;
}

double stod(std::string_view sv) noexcept
{
    using namespace boost::spirit;
    double val{};
    qi::parse(sv.begin(), sv.end(), qi::double_, val);
    return val;
}

} // namespace util
} // namespace toolbox
