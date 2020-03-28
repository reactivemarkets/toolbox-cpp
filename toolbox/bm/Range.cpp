// The Reactive C++ Toolbox.
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

#include "Range.hpp"

#include <toolbox/hdr/Histogram.hpp>

#include <cassert>

namespace toolbox::bm {
using namespace std;

BenchmarkRange::BenchmarkRange(HdrHistogram& hist, int first, int last) noexcept
: hist_{hist}
, first_{first}
, last_{last}
, start_{chrono::high_resolution_clock::now()}
{
    assert(first < last);
}

BenchmarkRange::~BenchmarkRange()
{
    const auto end = chrono::high_resolution_clock::now();
    const auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start_);
    const auto count = last_ - first_;
    // Record average.
    hist_.record_values(elapsed.count() / count, count);
}

} // namespace toolbox::bm
