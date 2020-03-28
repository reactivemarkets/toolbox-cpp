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

#include <toolbox/util/RobinHood.hpp>

#include <toolbox/bm.hpp>

#include <map>
#include <random>
#include <unordered_map>

// This benchmark is designed to measure random insertions and removals in a map of packed-sized
// data with a small keyspace (<100).

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

struct Elem {
    char data[1472];
};

vector<int> make_rand_data(int range, size_t count)
{
    random_device rd;
    mt19937 gen{rd()};
    uniform_int_distribution<> dis{1, range};

    vector<int> data;
    data.reserve(count);
    for (size_t i{0}; i < count; ++i) {
        data.push_back(dis(gen));
    }
    return data;
}

const int KeyRange{64};
const size_t RandCount{4096};

// Use the same random sequence for each benchmark.
const auto RandData = make_rand_data(KeyRange, RandCount);

TOOLBOX_BENCHMARK(std_map_insert_erase)
{
    map<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
            m.erase(i++ % RandData.size());
        }
    }
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_erase)
{
    unordered_map<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
            m.erase(i++ % RandData.size());
        }
    }
}

TOOLBOX_BENCHMARK(robin_map_insert_erase)
{
    RobinMap<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
            m.erase(i++ % RandData.size());
        }
    }
}

TOOLBOX_BENCHMARK(std_map_immutable)
{
    map<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
        }
    }
}

TOOLBOX_BENCHMARK(std_unordered_map_immutable)
{
    unordered_map<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
        }
    }
}

TOOLBOX_BENCHMARK(robin_map_immutable)
{
    RobinMap<int, Elem> m;

    size_t i{0};
    while (ctx) {
        for (auto _ : ctx.range(100)) {
            m[i++ % RandData.size()];
        }
    }
}

} // namespace
