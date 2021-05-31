// The Reactive C++ Toolbox.
// Copyright (C) 2013-2019 Swirly Cloud Limited
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

#include "Stream.hpp"

namespace toolbox {
inline namespace io {
using namespace std;

StreamBuf::~StreamBuf() = default;

StreamBuf::int_type StreamBuf::overflow(int_type c) noexcept
{
    if (c != traits_type::eof()) {
        auto buf = buf_.prepare(pcount_ + 1);
        pbase_ = buffer_cast<char*>(buf);
        pbase_[pcount_++] = c;
    }
    return c;
}

streamsize StreamBuf::xsputn(const char_type* s, streamsize count) noexcept
{
    auto buf = buf_.prepare(pcount_ + count);
    pbase_ = buffer_cast<char*>(buf);
    memcpy(pbase_ + pcount_, s, count);
    pcount_ += count;
    return count;
}

OStream::~OStream() = default;

} // namespace io
} // namespace toolbox
