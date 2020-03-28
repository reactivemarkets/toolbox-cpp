// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#include "Endian.hpp"

namespace toolbox {
inline namespace net {
using namespace std;

static_assert(bswap(uint16_t{0x0102}) == uint16_t{0x0201});
static_assert(bswap(int16_t{0x0102}) == int16_t{0x0201});
static_assert(bswap(uint32_t{0x01020304}) == uint32_t{0x04030201});
static_assert(bswap(int32_t{0x01020304}) == int32_t{0x04030201});
static_assert(bswap(uint64_t{0x0102030405060708}) == uint64_t{0x0807060504030201});
static_assert(bswap(int64_t{0x0102030405060708}) == int64_t{0x0807060504030201});

static_assert(ntoh(hton(uint16_t{0x0102})) == uint16_t{0x0102});
static_assert(ntoh(hton(int16_t{0x0102})) == int16_t{0x0102});
static_assert(ntoh(hton(uint32_t{0x01020304})) == uint32_t{0x01020304});
static_assert(ntoh(hton(int32_t{0x01020304})) == int32_t{0x01020304});
static_assert(ntoh(hton(uint64_t{0x0102030405060708})) == uint64_t{0x0102030405060708});
static_assert(ntoh(hton(int64_t{0x0102030405060708})) == int64_t{0x0102030405060708});

static_assert(ltoh(htol(uint16_t{0x0102})) == uint16_t{0x0102});
static_assert(ltoh(htol(int16_t{0x0102})) == int16_t{0x0102});
static_assert(ltoh(htol(uint32_t{0x01020304})) == uint32_t{0x01020304});
static_assert(ltoh(htol(int32_t{0x01020304})) == int32_t{0x01020304});
static_assert(ltoh(htol(uint64_t{0x0102030405060708})) == uint64_t{0x0102030405060708});
static_assert(ltoh(htol(int64_t{0x0102030405060708})) == int64_t{0x0102030405060708});

} // namespace net
} // namespace toolbox
