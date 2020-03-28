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

#ifndef TOOLBOX_UTIL_TOKENISER_HPP
#define TOOLBOX_UTIL_TOKENISER_HPP

#include <toolbox/util/String.hpp>

#include <algorithm>
#include <array>

namespace toolbox {
inline namespace util {

class Tokeniser {
  public:
    Tokeniser(std::string_view buf, std::string_view delims) noexcept { reset(buf, delims); }
    Tokeniser() noexcept { reset(""sv, ""sv); }
    ~Tokeniser() = default;

    // Copy.
    Tokeniser(const Tokeniser&) noexcept = default;
    Tokeniser& operator=(const Tokeniser&) noexcept = default;

    // Move.
    Tokeniser(Tokeniser&&) noexcept = default;
    Tokeniser& operator=(Tokeniser&&) noexcept = default;

    void reset(std::string_view buf, std::string_view delims) noexcept
    {
        buf_ = buf;
        delims_ = delims;
        i_ = buf_.cbegin();
        j_ = std::find_first_of(i_, buf_.cend(), delims_.cbegin(), delims_.cend());
    }
    /// Returns total bytes consumed.
    std::size_t consumed() const noexcept { return i_ - buf_.cbegin(); }
    /// Returns true if all bytes have been consumed.
    bool empty() const noexcept { return i_ == buf_.cend(); }
    /// Returns true if a delimiter was found in the remaining data.
    bool has_delim() const noexcept { return j_ != buf_.cend(); }
    std::string_view top() const noexcept { return buf_.substr(i_ - buf_.cbegin(), j_ - i_); }
    std::string_view next() noexcept
    {
        const auto tok = top();
        pop();
        return tok;
    }
    void pop() noexcept
    {
        if (j_ != buf_.cend()) {
            i_ = j_ + 1;
            j_ = std::find_first_of(i_, buf_.cend(), delims_.cbegin(), delims_.cend());
        } else {
            i_ = j_;
        }
    }

  private:
    std::string_view buf_;
    std::string_view delims_;
    // Assumption: std::string_view iterators are equivalent across copies of the std::string_view.
    std::string_view::const_iterator i_, j_;
};

template <typename FnT>
std::size_t parse_line(std::string_view buf, FnT fn)
{
    Tokeniser lines{buf, "\n"sv};
    while (lines.has_delim()) {
        fn(lines.top());
        lines.pop();
    }
    return lines.consumed();
}

template <std::size_t N>
using Row = std::array<std::string_view, N>;

template <std::size_t N>
void split(std::string_view line, std::string_view delims, Row<N>& row) noexcept
{
    Tokeniser toks{line, delims};
    for (auto& col : row) {
        if (toks.empty()) {
            break;
        }
        col = toks.top();
        toks.pop();
    }
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_TOKENISER_HPP
