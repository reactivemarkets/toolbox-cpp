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

#include "Timer.hpp"

#include <boost/test/unit_test.hpp>

namespace std::chrono {
template <typename RepT, typename PeriodT>
ostream& operator<<(ostream& os, duration<RepT, PeriodT> d)
{
    return os << d.count();
}
} // namespace std::chrono

namespace toolbox {
inline namespace io {
std::ostream& operator<<(std::ostream& os, const Timer& tmr)
{
    return os << tmr.id();
}
} // namespace io
} // namespace toolbox

using namespace std;
using namespace toolbox;

BOOST_AUTO_TEST_SUITE(TimerSuite)

BOOST_AUTO_TEST_CASE(TimerEmptyCase)
{
    Timer t;
    BOOST_TEST(t.empty());
    BOOST_TEST(!t);
    BOOST_TEST(t.id() == 0);
    BOOST_TEST(!t.pending());
}

BOOST_AUTO_TEST_CASE(TimerInsertCase)
{
    const auto now = MonoClock::now();
    TimerPool tp;
    TimerQueue tq{tp};

    auto fn = [](CyclTime now, Timer& tmr) {};
    Timer t = tq.insert(now + 2s, bind(&fn));
    BOOST_TEST(!t.empty());
    BOOST_TEST(t);
    BOOST_TEST(t.id() == 1);
    BOOST_TEST(t.pending());
    BOOST_TEST(t.expiry() == now + 2s);
    BOOST_TEST(t.interval() == 0s);
}

BOOST_AUTO_TEST_SUITE_END()
