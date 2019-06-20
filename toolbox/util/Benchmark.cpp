// The Reactive C++ Toolbox.
// Copyright (C) 2019 Reactive Markets Limited
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

#include <toolbox/sys/Options.hpp>

#include <random>
#include <regex>

namespace toolbox {
inline namespace benchmark {

Runnable::~Runnable() = default;

BenchmarkStore& BenchmarkStore::instance()
{
    static BenchmarkStore store;
    return store;
}

void BenchmarkStore::store(const char* name, Runnable& runnable)
{
    store_.insert_or_assign(name, &runnable);
}

void BenchmarkStore::list() const
{
    for (const auto& runnable : store_) {
        std::cout << runnable.first << '\n';
    }
}

void BenchmarkStore::run(const std::string& regex_str, bool randomise)
{
    std::vector<Runnable*> filtered;

    std::regex regex{regex_str};
    for (const auto& runnable : store_) {
        if (std::regex_search(runnable.first, regex)) {
            filtered.push_back(runnable.second);
        }
    }

    if (randomise) {
        auto rng = std::default_random_engine{};
        std::shuffle(std::begin(filtered), std::end(filtered), rng);
    }

    if (!filtered.empty()) {
        // Print header
        std::cout << "Benchmark  Average(ns)  Runs Time(us)\n";
        for (auto* test : filtered) {
            test->run();
        }
    }
}

BenchmarkStore::BenchmarkStore() = default;

namespace detail {
int main(int argc, char* argv[])
{
    std::string regex;
    bool list{false};
    bool randomise{false};

    Options opts{"Benchmark options [OPTIONS] [COMMAND]"};
    // clang-format off
    opts('f', "filter", Value{regex}, "Run only matches of regex")
        ('l', "list", Switch{list}, "List available benchmarks")
        ('h', "help", Help{})
        ('r', "random", Switch{randomise}, "Run tests in random order")
    ;
    // clang-format on

    try {
        opts.parse(argc, argv);
    } catch (const std::runtime_error& ex) {
        std::cerr << opts;
        return 1;
    }

    auto& store = BenchmarkStore::instance();
    if (list) {
        store.list();
        return 0;
    }

    store.run(regex, randomise);
    return 0;
}
} // namespace detail

} // namespace benchmark
} // namespace toolbox
