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

#ifndef TOOLBOX_UTIL_TOKENISER_HPP
#define TOOLBOX_UTIL_TOKENISER_HPP

#include <algorithm>
#include <array>
#include <string_view>

namespace toolbox {
inline namespace util {
using sv = std::string_view;

class Tokeniser {    
  public:
    constexpr Tokeniser(std::string_view buf, std::string_view delims) noexcept
    {
        reset(buf, delims);
    }
    constexpr Tokeniser() noexcept { reset(sv{""}, sv{""}); }
    constexpr ~Tokeniser() = default;

    // Copy.
    constexpr Tokeniser(const Tokeniser&) noexcept = default;
    constexpr Tokeniser& operator=(const Tokeniser&) noexcept = default;

    // Move.
    constexpr Tokeniser(Tokeniser&&) noexcept = default;
    constexpr Tokeniser& operator=(Tokeniser&&) noexcept = default;

    constexpr void reset(std::string_view buf, std::string_view delims) noexcept
    {
        buf_ = buf;
        delims_ = delims;
        i_ = buf_.cbegin();
        j_ = std::find_first_of(i_, buf_.cend(), delims_.cbegin(), delims_.cend());
    }
    /// Returns total bytes consumed.
    constexpr std::size_t consumed() const noexcept { return i_ - buf_.cbegin(); }
    /// Returns true if all bytes have been consumed.
    constexpr bool empty() const noexcept { return i_ == buf_.cend(); }
    /// Returns true if a delimiter was found in the remaining data.
    constexpr bool has_delim() const noexcept { return j_ != buf_.cend(); }
    constexpr std::string_view top() const noexcept
    {
        return buf_.substr(i_ - buf_.cbegin(), j_ - i_);
    }
    constexpr std::string_view next() noexcept
    {
        const auto tok = top();
        pop();
        return tok;
    }
    constexpr void pop() noexcept
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
constexpr std::size_t parse_line(std::string_view buf, FnT fn)
{
    Tokeniser lines{buf, sv{"\n"}};
    while (lines.has_delim()) {
        fn(lines.top());
        lines.pop();
    }
    return lines.consumed();
}

template <std::size_t N>
using Row = std::array<std::string_view, N>;

template <std::size_t N>
constexpr void split(std::string_view line, std::string_view delims, Row<N>& row) noexcept
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
