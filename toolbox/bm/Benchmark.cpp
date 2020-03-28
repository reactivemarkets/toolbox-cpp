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

#include "Benchmark.hpp"

#include <toolbox/bm/Suite.hpp>

#include <toolbox/util/Options.hpp>

#include <iostream>
#include <random>
#include <regex>

namespace toolbox::bm {
using namespace std;
namespace {
class BenchmarkStore {
  public:
    // Copy.
    BenchmarkStore(const BenchmarkStore&) = delete;
    BenchmarkStore& operator=(const BenchmarkStore&) = delete;

    // Move.
    BenchmarkStore(BenchmarkStore&&) = delete;
    BenchmarkStore& operator=(BenchmarkStore&&) = delete;

    static BenchmarkStore& instance()
    {
        static BenchmarkStore store;
        return store;
    }
    void list(ostream& os) const
    {
        for (const auto& runnable : store_) {
            os << runnable.first << '\n';
        }
    }
    void run(ostream& os, const string& regex_str, bool randomise)
    {
        vector<Benchmark*> filtered;

        regex regex{regex_str};
        for (const auto& bm : store_) {
            if (regex_search(bm.first, regex)) {
                filtered.push_back(bm.second);
            }
        }

        if (randomise) {
            default_random_engine rng{};
            shuffle(begin(filtered), end(filtered), rng);
        }

        if (!filtered.empty()) {
            BenchmarkSuite suite{os, 1000.0};
            for (auto* bm : filtered) {
                suite.run(bm->name, bm->fn);
            }
        }
    }
    void store(const char* name, Benchmark& bm) { store_.insert_or_assign(name, &bm); }

  private:
    BenchmarkStore() = default;

    map<const char*, Benchmark*> store_;
};
} // namespace

Benchmark::Benchmark(const char* name, void (*fn)(BenchmarkCtx&))
: name{name}
, fn{fn}
{
    BenchmarkStore::instance().store(name, *this);
}

namespace detail {
int main(int argc, char* argv[])
{
    int ret = 1;
    try {
        string regex;
        bool list{false};
        bool randomise{false};

        Options opts{"benchmark options [options]"};
        // clang-format off
        opts('f', "filter", Value{regex}, "run benchmarks matching regex")
            ('l', "list", Switch{list}, "list available benchmarks")
            ('h', "help", Help{})
            ('r', "random", Switch{randomise}, "run benchmarks in random order")
            ;
        // clang-format on

        opts.parse(argc, argv);

        auto& store = BenchmarkStore::instance();
        if (list) {
            store.list(cout);
            return 0;
        }
        store.run(cout, regex, randomise);
        ret = 0;
    } catch (const exception& e) {
        cerr << "error: " << e.what();
    }
    return ret;
}
} // namespace detail
} // namespace toolbox::bm
