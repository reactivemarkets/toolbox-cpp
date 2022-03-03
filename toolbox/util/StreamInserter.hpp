// The Reactive C++ Toolbox.
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

#ifndef TOOLBOX_UTIL_STREAMINSERTER_HPP
#define TOOLBOX_UTIL_STREAMINSERTER_HPP

#include <concepts>
#include <iosfwd>
#include <utility>

namespace toolbox {
inline namespace util {

/// StreamInserter is a class template designed to simplify I/O manipulator and stream inserter
/// implementations by reducing boiler-plate code.
///
// clang-format off
template <typename OStreamT, typename FnT>
requires std::invocable<FnT, OStreamT&>
// clang-format on
class StreamInserter {
    friend OStreamT& operator<<(OStreamT& os, const StreamInserter& si)
    {
        si.fn_(os);
        return os;
    }

  public:
    explicit StreamInserter(FnT fn)
    : fn_{std::move(fn)}
    {
    }
    ~StreamInserter() = default;

    // Copy.
    StreamInserter(const StreamInserter&) = delete;
    StreamInserter& operator=(const StreamInserter&) = delete;

    // Move.
    StreamInserter(StreamInserter&&) noexcept = default;
    StreamInserter& operator=(StreamInserter&&) noexcept = delete;

  private:
    FnT fn_;
};

/// Returns a StreamInserter object, automatically deducing functor type from arguments.
template <typename OStreamT, typename FnT>
auto make_stream_inserter(FnT fn)
{
    return StreamInserter<OStreamT, FnT>{std::move(fn)};
}

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_STREAMINSERTER_HPP
