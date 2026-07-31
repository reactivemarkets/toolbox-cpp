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

#include <toolbox/util/Random.hpp>
#include <toolbox/util/RobinHood.hpp>

#include <toolbox/bm.hpp>

#include <boost/unordered/unordered_flat_map.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <stdexcept>
#include <unordered_map>
#include <vector>

// Compares toolbox::RobinMap against boost::unordered_flat_map (the proposed replacement) and
// std::unordered_map (a stable baseline) across successful and unsuccessful lookups, insertion
// into an initially empty map, steady-state churn, and full iteration.
//
// Keys are distinct integers, shuffled once per run and shared by every benchmark and
// implementation, so all three implementations always compare against identical inputs while
// varying between runs. Setup happens outside the timed ranges, results are consumed so
// operations cannot be optimised away, and final cardinality is checked.
//
// Benchmark names follow <implementation>_<operation>_<entries>_<value shape>.

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

/// Small mapped value representing an id, index, or pointer-sized payload.
using SmallValue = int64_t;

/// Large mapped value comparable to a packet-sized payload.
struct LargeValue {
    char data[1472];
};

/// Number of passes over the key set per timed batch, to amortise timer overhead.
constexpr size_t Sweeps{8};

/// Returns count * 2 distinct keys, shuffled with the toolbox's shared RNG: the first count keys
/// populate each map, and the remaining count keys are guaranteed-absent probes.
vector<int> make_keys(size_t count)
{
    vector<int> keys(count * 2);
    ranges::iota(keys, 0);
    ranges::shuffle(keys, mt19937_64_rng());
    return keys;
}

const auto Keys64 = make_keys(64);
const auto Keys4096 = make_keys(4096);

/// Populates m with the first half of keys; setup only, outside any timed range.
template <typename MapT>
void populate(MapT& m, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    for (size_t i{0}; i < count; ++i) {
        m.try_emplace(keys[i]);
    }
}

/// Successful lookups on a fully populated map that never mutates.
template <typename MapT>
void run_find_hit(bm::Context& ctx, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    MapT m;
    populate(m, keys);

    size_t hits{0};
    size_t ops{0};
    while (ctx) {
        {
            // Named so its lifetime spans the sweeps: batch records the elapsed time per
            // operation when it leaves scope.
            auto batch{ctx.range(static_cast<int>(Sweeps * count))};
            for (size_t sweep{0}; sweep < Sweeps; ++sweep) {
                for (size_t i{0}; i < count; ++i) {
                    if (m.find(keys[i]) != m.end()) {
                        ++hits;
                    }
                }
                // Stop the optimiser caching lookups across sweeps of the same keys.
                bm::clobber_memory();
            }
        }
        ops += Sweeps * count;
        bm::do_not_optimise(hits);
    }
    if (m.size() != count || hits != ops) {
        throw runtime_error{"find hit benchmark invariant broken"};
    }
}

/// Unsuccessful lookups on a fully populated map that never mutates.
template <typename MapT>
void run_find_miss(bm::Context& ctx, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    MapT m;
    populate(m, keys);

    size_t hits{0};
    while (ctx) {
        {
            auto batch{ctx.range(static_cast<int>(Sweeps * count))};
            for (size_t sweep{0}; sweep < Sweeps; ++sweep) {
                for (size_t i{0}; i < count; ++i) {
                    if (m.find(keys[count + i]) != m.end()) {
                        ++hits;
                    }
                }
                bm::clobber_memory();
            }
        }
        bm::do_not_optimise(hits);
    }
    if (m.size() != count || hits != 0) {
        throw runtime_error{"find miss benchmark invariant broken"};
    }
}

/// Insertion into an initially empty map. The timed region covers growth from empty to count
/// entries, including rehashes; construction and destruction of the map are not timed.
template <typename MapT>
void run_insert(bm::Context& ctx, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    while (ctx) {
        MapT m;
        for (auto i : ctx.range(static_cast<int>(count))) {
            m.try_emplace(keys[static_cast<size_t>(i)]);
        }
        if (m.size() != count) {
            throw runtime_error{"insert benchmark invariant broken"};
        }
    }
}

/// Steady-state churn: each operation erases a present key and inserts an absent key, sliding a
/// count-sized window over the key universe so cardinality never changes.
template <typename MapT>
void run_churn(bm::Context& ctx, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    MapT m;
    populate(m, keys);

    size_t erased{0};
    size_t ops{0};
    size_t erase_idx{0};
    size_t insert_idx{count};
    while (ctx) {
        for ([[maybe_unused]] auto _ : ctx.range(static_cast<int>(count))) {
            erased += m.erase(keys[erase_idx]);
            m.try_emplace(keys[insert_idx]);
            // Wrap with predictable branches: modulo would add a division per timed operation.
            if (++erase_idx == keys.size()) {
                erase_idx = 0;
            }
            if (++insert_idx == keys.size()) {
                insert_idx = 0;
            }
        }
        ops += count;
        bm::do_not_optimise(erased);
    }
    if (m.size() != count || erased != ops) {
        throw runtime_error{"churn benchmark invariant broken"};
    }
}

/// Full iteration over a fully populated map, consuming a checksum of keys and values.
template <typename MapT>
void run_iterate(bm::Context& ctx, const vector<int>& keys)
{
    const auto count{keys.size() / 2};
    MapT m;
    for (size_t i{0}; i < count; ++i) {
        m.try_emplace(keys[i], static_cast<typename MapT::mapped_type>(keys[i]));
    }

    uint64_t sum{0};
    while (ctx) {
        {
            auto batch{ctx.range(static_cast<int>(Sweeps * count))};
            for (size_t sweep{0}; sweep < Sweeps; ++sweep) {
                for (const auto& kv : m) {
                    sum += static_cast<uint64_t>(kv.first) + static_cast<uint64_t>(kv.second);
                }
                bm::clobber_memory();
            }
        }
        bm::do_not_optimise(sum);
    }
    if (m.size() != count) {
        throw runtime_error{"iterate benchmark invariant broken"};
    }
}

// --- find_hit: successful lookups on a fully populated map, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_find_hit_64_small)
{
    run_find_hit<RobinMap<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_hit_64_small)
{
    run_find_hit<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_hit_64_small)
{
    run_find_hit<unordered_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_find_hit_4096_small)
{
    run_find_hit<RobinMap<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_hit_4096_small)
{
    run_find_hit<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_hit_4096_small)
{
    run_find_hit<unordered_map<int, SmallValue>>(ctx, Keys4096);
}

// --- find_miss: unsuccessful lookups on a fully populated map, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_find_miss_64_small)
{
    run_find_miss<RobinMap<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_miss_64_small)
{
    run_find_miss<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_miss_64_small)
{
    run_find_miss<unordered_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_find_miss_4096_small)
{
    run_find_miss<RobinMap<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_miss_4096_small)
{
    run_find_miss<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_miss_4096_small)
{
    run_find_miss<unordered_map<int, SmallValue>>(ctx, Keys4096);
}

// --- iterate: full pass over a fully populated map, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_iterate_64_small)
{
    run_iterate<RobinMap<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_iterate_64_small)
{
    run_iterate<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_iterate_64_small)
{
    run_iterate<unordered_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_iterate_4096_small)
{
    run_iterate<RobinMap<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_iterate_4096_small)
{
    run_iterate<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_iterate_4096_small)
{
    run_iterate<unordered_map<int, SmallValue>>(ctx, Keys4096);
}

// --- insert: fill an initially empty map, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_insert_64_small)
{
    run_insert<RobinMap<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_64_small)
{
    run_insert<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_64_small)
{
    run_insert<unordered_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_insert_4096_small)
{
    run_insert<RobinMap<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_4096_small)
{
    run_insert<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_4096_small)
{
    run_insert<unordered_map<int, SmallValue>>(ctx, Keys4096);
}

// --- insert: fill an initially empty map, large mapped value ---

TOOLBOX_BENCHMARK(robin_map_insert_64_large)
{
    run_insert<RobinMap<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_64_large)
{
    run_insert<boost::unordered_flat_map<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_64_large)
{
    run_insert<unordered_map<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_insert_4096_large)
{
    run_insert<RobinMap<int, LargeValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_4096_large)
{
    run_insert<boost::unordered_flat_map<int, LargeValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_4096_large)
{
    run_insert<unordered_map<int, LargeValue>>(ctx, Keys4096);
}

// --- churn: steady-state erase-and-insert, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_churn_64_small)
{
    run_churn<RobinMap<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_64_small)
{
    run_churn<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_64_small)
{
    run_churn<unordered_map<int, SmallValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_churn_4096_small)
{
    run_churn<RobinMap<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_4096_small)
{
    run_churn<boost::unordered_flat_map<int, SmallValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_4096_small)
{
    run_churn<unordered_map<int, SmallValue>>(ctx, Keys4096);
}

// --- churn: steady-state erase-and-insert, large mapped value ---

TOOLBOX_BENCHMARK(robin_map_churn_64_large)
{
    run_churn<RobinMap<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_64_large)
{
    run_churn<boost::unordered_flat_map<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_64_large)
{
    run_churn<unordered_map<int, LargeValue>>(ctx, Keys64);
}

TOOLBOX_BENCHMARK(robin_map_churn_4096_large)
{
    run_churn<RobinMap<int, LargeValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_4096_large)
{
    run_churn<boost::unordered_flat_map<int, LargeValue>>(ctx, Keys4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_4096_large)
{
    run_churn<unordered_map<int, LargeValue>>(ctx, Keys4096);
}

} // namespace
