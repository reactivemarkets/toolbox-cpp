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

#include "Suite.hpp"

#include <toolbox/hdr/Utility.hpp>

#include <boost/io/ios_state.hpp>

#include <iomanip>

namespace toolbox::bm {
using namespace std;

BenchmarkSuite::BenchmarkSuite(std::ostream& os, double value_scale)
: os_{os}
, value_scale_{value_scale}
{
    boost::io::ios_all_saver all_saver{os};

    // clang-format off
    os << left << setw(45) << "NAME"
       << right << setw(15) << "COUNT"
       << right << setw(10) << "MIN"
       << right << setw(10) << "%50"
       << right << setw(10) << "%95"
       << right << setw(10) << "%99"
       << right << setw(10) << "%99.9"
       << right << setw(10) << "%99.99"
       << endl;
    // clang-format on

    // Separator.
    os << setw(120) << setfill('-') << '-' << setfill(' ') << endl;
}

void BenchmarkSuite::report(const char* name, HdrHistogram& h)
{
    boost::io::ios_all_saver all_saver{os_};

    // clang-format off
    os_ << left << setw(45) << name
        << right << setw(15) << h.total_count()
        << right << setw(10) << h.min() / value_scale_
        << right << setw(10) << value_at_percentile(h, 50) / value_scale_
        << right << setw(10) << value_at_percentile(h, 95) / value_scale_
        << right << setw(10) << value_at_percentile(h, 99) / value_scale_
        << right << setw(10) << value_at_percentile(h, 99.9) / value_scale_
        << right << setw(10) << value_at_percentile(h, 99.99) / value_scale_
        << endl;
    // clang-format on
}

} // namespace toolbox::bm
