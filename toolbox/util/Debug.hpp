// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#ifndef TOOLBOX_UTIL_DEBUG_HPP
#define TOOLBOX_UTIL_DEBUG_HPP

#include <toolbox/Config.h>

#include <iomanip>
#include <iostream>

namespace toolbox {
inline namespace util {

class hex_dump {
  public:
    enum class Mode { ALL = 0, NON_PRINTABLE };

    template <typename Type>
    explicit hex_dump(const Type* obj, int size = sizeof(Type), Mode mode = Mode::ALL)
    : obj_{reinterpret_cast<const unsigned char*>(obj)}
    , size_{size}
    , mode_{mode}
    {
    }

    friend std::ostream& operator<<(std::ostream& out, const hex_dump& dump)
    {
        if (dump.obj_) {
            auto begin = dump.obj_;
            const auto end = begin + dump.size_;
            out << std::hex << std::setfill('0');
            while (begin != end) {
                const int ch = +*begin;
                if (isprint(ch) && dump.mode_ == Mode::NON_PRINTABLE) {
                    out << ' ' << static_cast<char>(ch);
                } else {
                    out << " 0x" << std::setw(2) << +ch;
                }
                ++begin;
            }
            out << std::dec << std::setfill(' ');
        }
        return out;
    }

  private:
    const uint8_t* obj_;
    int size_;
    Mode mode_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_DEBUG_HPP
