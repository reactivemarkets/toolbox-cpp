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

#ifndef TOOLBOX_BM_RECORD
#define TOOLBOX_BM_RECORD

#include <toolbox/Config.h>

#include <chrono>

namespace toolbox {
inline namespace hdr {
class HdrHistogram;
} // namespace hdr
} // namespace toolbox

namespace toolbox::bm {

/// The BenchmarkRecord class records the time elapsed during object lifetime,
/// i.e., between construction and destruction.
/// The elapsed time is recorded in the HdrHistogram object during destruction.
class TOOLBOX_API BenchmarkRecord {
  public:
    BenchmarkRecord(HdrHistogram& hist, int count = 1) noexcept;
    ~BenchmarkRecord();

    // Copy.
    BenchmarkRecord(const BenchmarkRecord&) = delete;
    BenchmarkRecord& operator=(const BenchmarkRecord&) = delete;

    // Move.
    BenchmarkRecord(BenchmarkRecord&&) = delete;
    BenchmarkRecord& operator=(BenchmarkRecord&&) = delete;

  private:
    HdrHistogram& hist_;
    const int count_;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
};

} // namespace toolbox::bm

#endif // TOOLBOX_BM_RECORD
