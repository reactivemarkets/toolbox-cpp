// The Reactive C++ Toolbox.
// Copyright (C) 2021 Reactive Markets Limited
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

#include "Record.hpp"

#include <toolbox/hdr/Histogram.hpp>

#include <cassert>

namespace toolbox::bm {
using namespace std;

BenchmarkRecord::BenchmarkRecord(Histogram& hist, int count) noexcept
: hist_{hist}
, count_{count}
, start_{chrono::high_resolution_clock::now()}
{
    assert(count >= 1);
}

BenchmarkRecord::~BenchmarkRecord()
{
    const auto end = chrono::high_resolution_clock::now();
    const auto elapsed = chrono::duration_cast<chrono::nanoseconds>(end - start_);
    // Record average.
    hist_.record_values(elapsed.count() / count_, count_);
}

} // namespace toolbox::bm
