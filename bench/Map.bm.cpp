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

#include <boost/functional/hash.hpp>
#include <boost/unordered/unordered_flat_map.hpp>

#include <cstdint>
#include <cstring>
#include <ranges>
#include <unordered_map>
#include <vector>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

/// Fixed-size character-array key.
struct Key {
    friend size_t hash_value(const Key& key)
    {
        size_t hash{0};
        for (auto c : key.data) {
            boost::hash_combine(hash, c);
        }
        return hash;
    }

    char data[64]{};

    explicit Key(uint64_t value) noexcept { std::memcpy(data, &value, sizeof(value)); }

    bool operator==(const Key&) const = default;
};

/// Small mapped value representing an id, index, or pointer-sized payload.
using SmallValue = int64_t;

/// Larger counterpart to Key, with a distinct id in its first eight bytes.
struct LargeValue {
    char data[1472]{};

    explicit LargeValue(uint64_t value) noexcept { std::memcpy(data, &value, sizeof(value)); }

    explicit operator uint64_t() const noexcept
    {
        uint64_t value{};
        std::memcpy(&value, data, sizeof(value));
        return value;
    }
};

/// Returns count distinct keys, starting at offset and shuffled with the toolbox's shared RNG.
vector<Key> make_keys(size_t offset, size_t count)
{
    vector<Key> keys;
    keys.reserve(count);
    for (size_t i{0}; i < count; ++i) {
        keys.emplace_back(offset + i);
    }
    ranges::shuffle(keys, mt19937_64_rng());
    return keys;
}

const auto present_keys_4096 = make_keys(0, 4096);
const auto absent_keys_4096 = make_keys(4096, 4096);

/// Returns a shuffled probe sequence with an equal number of present and absent keys.
vector<Key> make_find_probes(const vector<Key>& present_keys, const vector<Key>& absent_keys)
{
    vector<Key> probes;
    probes.reserve(present_keys.size() + absent_keys.size());
    probes.insert(probes.end(), present_keys.begin(), present_keys.end());
    probes.insert(probes.end(), absent_keys.begin(), absent_keys.end());
    ranges::shuffle(probes, mt19937_64_rng());
    return probes;
}

const auto find_probes_4096 = make_find_probes(present_keys_4096, absent_keys_4096);

/// Populates m with present keys; setup only, outside any timed range.
template <typename MapT>
void populate(MapT& m, const vector<Key>& present_keys)
{
    const auto count{present_keys.size()};
    for (size_t i{0}; i < count; ++i) {
        m.try_emplace(present_keys[i], static_cast<uint64_t>(i));
    }
}

/// Shuffled 50/50 successful and unsuccessful lookups on a fully populated map that never mutates.
template <typename MapT>
void run_find_50_50(bm::Context& ctx, const vector<Key>& present_keys, const vector<Key>& probes)
{
    MapT m;
    populate(m, present_keys);

    size_t hits{0};
    while (ctx) {
        for (auto i : ctx.range(static_cast<int>(probes.size()))) {
            hits += m.find(probes[static_cast<size_t>(i)]) != m.end();
        }
        bm::do_not_optimise(hits);
    }
}

/// Insertion into an initially empty, pre-reserved map. The timed region covers each insertion but
/// excludes table allocation and rehashing; per-element allocation remains part of the measurement.
template <typename MapT>
void run_insert(bm::Context& ctx, const vector<Key>& keys)
{
    const auto count{keys.size()};
    while (ctx) {
        MapT m;
        m.reserve(count);
        for (auto i : ctx.range(static_cast<int>(count))) {
            m.try_emplace(keys[static_cast<size_t>(i)], static_cast<uint64_t>(i));
        }
    }
}

/// Steady-state churn: each batch replaces all present keys with absent keys, then reverses that
/// exchange in the next batch, so cardinality never changes.
template <typename MapT>
void run_churn(bm::Context& ctx, const vector<Key>& present_keys, const vector<Key>& absent_keys)
{
    const auto count{present_keys.size()};
    MapT m;
    populate(m, present_keys);

    size_t erased{0};
    bool erase_present{true};
    while (ctx) {
        const auto& erase_keys = erase_present ? present_keys : absent_keys;
        const auto& insert_keys = erase_present ? absent_keys : present_keys;
        for (auto i : ctx.range(static_cast<int>(count))) {
            const auto key_idx = static_cast<size_t>(i);
            erased += m.erase(erase_keys[key_idx]);
            m.try_emplace(insert_keys[key_idx], static_cast<uint64_t>(key_idx));
        }
        erase_present = !erase_present;
        bm::do_not_optimise(erased);
    }
}

/// Full iteration over a fully populated map, consuming every mapped value.
template <typename MapT>
void run_iterate(bm::Context& ctx, const vector<Key>& keys)
{
    const auto count{keys.size()};
    MapT m;
    for (size_t i{0}; i < count; ++i) {
        m.try_emplace(keys[i], static_cast<uint64_t>(i));
    }
    bm::clobber_memory();

    uint64_t sum{0};
    while (ctx) {
        {
            auto batch{ctx.range(static_cast<int>(count))};
            for (const auto& kv : m) {
                bm::do_not_optimise(kv.second);
                sum += static_cast<uint64_t>(kv.second);
            }
        }
        bm::do_not_optimise(sum);
    }
}

// --- find_50_50: shuffled successful and unsuccessful lookups on a fully populated map ---

TOOLBOX_BENCHMARK(robin_map_find_50_50_4096_small)
{
    run_find_50_50<RobinMap<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                find_probes_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_50_50_4096_small)
{
    run_find_50_50<boost::unordered_flat_map<Key, SmallValue, boost::hash<Key>>>(
        ctx, present_keys_4096, find_probes_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_50_50_4096_small)
{
    run_find_50_50<unordered_map<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                     find_probes_4096);
}

TOOLBOX_BENCHMARK(robin_map_find_50_50_4096_large)
{
    run_find_50_50<RobinMap<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                find_probes_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_find_50_50_4096_large)
{
    run_find_50_50<boost::unordered_flat_map<Key, LargeValue, boost::hash<Key>>>(
        ctx, present_keys_4096, find_probes_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_find_50_50_4096_large)
{
    run_find_50_50<unordered_map<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                     find_probes_4096);
}

// --- iterate: full pass over a fully populated map ---

TOOLBOX_BENCHMARK(robin_map_iterate_4096_small)
{
    run_iterate<RobinMap<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_iterate_4096_small)
{
    run_iterate<boost::unordered_flat_map<Key, SmallValue, boost::hash<Key>>>(ctx,
                                                                              present_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_iterate_4096_small)
{
    run_iterate<unordered_map<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

TOOLBOX_BENCHMARK(robin_map_iterate_4096_large)
{
    run_iterate<RobinMap<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_iterate_4096_large)
{
    run_iterate<boost::unordered_flat_map<Key, LargeValue, boost::hash<Key>>>(ctx,
                                                                              present_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_iterate_4096_large)
{
    run_iterate<unordered_map<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

// --- insert: fill an initially empty map, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_insert_4096_small)
{
    run_insert<RobinMap<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_4096_small)
{
    run_insert<boost::unordered_flat_map<Key, SmallValue, boost::hash<Key>>>(ctx,
                                                                             present_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_4096_small)
{
    run_insert<unordered_map<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

// --- insert: fill an initially empty map, large mapped value ---

TOOLBOX_BENCHMARK(robin_map_insert_4096_large)
{
    run_insert<RobinMap<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_insert_4096_large)
{
    run_insert<boost::unordered_flat_map<Key, LargeValue, boost::hash<Key>>>(ctx,
                                                                             present_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_insert_4096_large)
{
    run_insert<unordered_map<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096);
}

// --- churn: steady-state erase-and-insert, small mapped value ---

TOOLBOX_BENCHMARK(robin_map_churn_4096_small)
{
    run_churn<RobinMap<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                           absent_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_4096_small)
{
    run_churn<boost::unordered_flat_map<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                            absent_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_4096_small)
{
    run_churn<unordered_map<Key, SmallValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                absent_keys_4096);
}

// --- churn: steady-state erase-and-insert, large mapped value ---

TOOLBOX_BENCHMARK(robin_map_churn_4096_large)
{
    run_churn<RobinMap<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                           absent_keys_4096);
}

TOOLBOX_BENCHMARK(boost_flat_map_churn_4096_large)
{
    run_churn<boost::unordered_flat_map<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                            absent_keys_4096);
}

TOOLBOX_BENCHMARK(std_unordered_map_churn_4096_large)
{
    run_churn<unordered_map<Key, LargeValue, boost::hash<Key>>>(ctx, present_keys_4096,
                                                                absent_keys_4096);
}

} // namespace
