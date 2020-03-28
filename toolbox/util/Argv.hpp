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

#ifndef TOOLBOX_UTIL_ARGV_HPP
#define TOOLBOX_UTIL_ARGV_HPP

#include <toolbox/Config.h>

#include <optional>
#include <string_view>

namespace toolbox {
inline namespace util {

/// Utility class that allows an argv array to be treated as a sequence.
class ArgvSequence {
  public:
    using ConstIterator = const char* const*;
    constexpr ArgvSequence(int argc, const char* const argv[])
    : begin_{argv}
    , end_{argv + argc}
    {
    }
    ~ArgvSequence() = default;

    // Copy.
    constexpr ArgvSequence(const ArgvSequence&) noexcept = default;
    constexpr ArgvSequence& operator=(const ArgvSequence&) noexcept = default;

    // Move.
    constexpr ArgvSequence(ArgvSequence&&) noexcept = default;
    constexpr ArgvSequence& operator=(ArgvSequence&&) noexcept = default;

    constexpr ConstIterator begin() const noexcept { return begin_; }
    constexpr ConstIterator end() const noexcept { return end_; }

  private:
    ConstIterator begin_, end_;
};

class TOOLBOX_API ArgvLexer {
  public:
    using ConstIterator = ArgvSequence::ConstIterator;
    ArgvLexer(ConstIterator begin, ConstIterator end) noexcept
    : it_{begin}
    , end_{end}
    {
        next();
    }
    explicit ArgvLexer(ArgvSequence argv) noexcept
    : ArgvLexer{argv.begin(), argv.end()}
    {
    }
    ArgvLexer(int argc, const char* const argv[]) noexcept
    : ArgvLexer{ArgvSequence{argc, argv}}
    {
    }
    ~ArgvLexer();

    // Copy.
    ArgvLexer(const ArgvLexer&) noexcept = default;
    ArgvLexer& operator=(const ArgvLexer&) noexcept = default;

    // Move.
    ArgvLexer(ArgvLexer&&) noexcept = default;
    ArgvLexer& operator=(ArgvLexer&&) noexcept = default;

    bool empty() const noexcept { return it_ == end_; }

    std::string_view arg() const noexcept { return *it_; }
    std::string_view opt() const noexcept { return opt_; }

    void pop() noexcept
    {
        ++it_;
        next();
    }
    /// Use this pop overload when the current option is a switch.
    /// Throw std::runtime_error if the switch had an attached value.
    void pop_switch();

    /// Use this pop overload when the current option expects a value.
    /// Throw std::runtime_error if no value exists.
    std::string_view pop_value();

  private:
    void next() noexcept;

    ConstIterator it_, end_;
    std::string_view opt_;
    std::optional<std::string_view> val_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_ARGV_HPP
