// The Reactive C++ Toolbox.
// Copyright (C) 2025 Reactive Markets Limited
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

#include <toolbox/net/Endpoint.hpp>
#include <toolbox/util/Random.hpp>
#include <toolbox/util/Stream.hpp>
#include <toolbox/bm.hpp>

#include <algorithm>
#include <cstddef>
#include <vector>
#include <limits>

#include <netinet/in.h>

TOOLBOX_BENCHMARK_MAIN

using namespace std;
using namespace toolbox;

namespace {

template<size_t N>
OStream<N>& make_stream() {
    static OStream<N> stream{nullptr};

    auto storage = OStream<N>::make_storage();

    // make sure all allocated memory is backed by physical pages,
    // so that no page faults occur during our benchmarks.
    unsigned char* p = static_cast<unsigned char*>(storage.get());
    std::fill(p, p+N, 0);

    stream.set_storage(std::move(storage));
    return stream;
}

std::vector<sockaddr_in> generate_random_ipv4_addresses(size_t N) {
    std::vector<sockaddr_in> ret;
    ret.reserve(N);

    for (size_t i = 0; i < N; i++) {
        sockaddr_in s;
        s.sin_family = AF_INET;
        s.sin_port = htons(randint<in_port_t>(0, std::numeric_limits<in_port_t>::max()));
        s.sin_addr = in_addr{randint<uint32_t>(0, std::numeric_limits<uint32_t>::max())};
        ret.push_back(s);
    }

    return ret;
}

OStream<32768>& os = make_stream<32768>();
std::vector<sockaddr_in> rand_ipv4_addresses = generate_random_ipv4_addresses(1024);

ostream& write_ipv4_libc(const sockaddr_in& sa)
{
    char buf[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &toolbox::remove_const(sa).sin_addr, buf, sizeof(buf));
    return os << buf << ':' << ntohs(sa.sin_port);
}

TOOLBOX_BENCHMARK(format_ipv4_endpoint_libc)
{
    os.reset();
    while (ctx) {
        for (auto i : ctx.range(rand_ipv4_addresses.size())) {
            auto ip_addr = rand_ipv4_addresses[i];
            write_ipv4_libc(ip_addr);
        }
    }
}

TOOLBOX_BENCHMARK(format_ipv4_endpoint)
{
    os.reset();
    while (ctx) {
        for (auto i : ctx.range(rand_ipv4_addresses.size())) {
            auto ip_addr = rand_ipv4_addresses[i];
            os << ip_addr;
        }
    }
}

} // namespace
