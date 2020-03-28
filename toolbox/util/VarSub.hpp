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

#ifndef TOOLBOX_UTIL_VARSUB_HPP
#define TOOLBOX_UTIL_VARSUB_HPP

#include <toolbox/Config.h>

#include <functional>
#include <set>
#include <string>

namespace toolbox {
inline namespace util {

TOOLBOX_API std::string get_env(const std::string& name);

/// Variable substitution.
class TOOLBOX_API VarSub {
  public:
    explicit VarSub(std::function<std::string(const std::string&)> fn = get_env)
    : fn_{std::move(fn)}
    {
    }
    ~VarSub();

    // Copy.
    VarSub(const VarSub&);
    VarSub& operator=(const VarSub&);

    // Move.
    VarSub(VarSub&&);
    VarSub& operator=(VarSub&&);

    void operator()(std::string& s) const { substitute(s, std::string::npos, 0); }

  private:
    /// Substitute variables.
    ///
    /// \param s The string to substitute.
    ///
    /// \param i Position of opening brace or std::string::npos if top-level.
    ///
    /// \param j Starting position of search. The search space is [j, size).
    ///
    /// \param outer Set of names seen at the same position in outer level.
    ///
    /// \return true if closing brace was found for brace at position i.
    bool substitute(std::string& s, const std::size_t i, std::size_t j,
                    std::set<std::string>* outer = nullptr) const;

    std::function<std::string(const std::string&)> fn_;
};

} // namespace util
} // namespace toolbox

#endif // TOOLBOX_UTIL_VARSUB_HPP
