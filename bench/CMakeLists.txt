# The Reactive C++ Toolbox.
# Copyright (C) 2013-2019 Swirly Cloud Limited
# Copyright (C) 2020 Reactive Markets Limited
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set(targets
  tb-map-bench
  tb-ryu-bench
  tb-time-bench
  tb-timer-bench
  tb-util-bench)

add_custom_target(tb-bench DEPENDS ${targets})

add_executable(tb-map-bench Map.bm.cpp)
target_link_libraries(tb-map-bench ${tb_bm_LIBRARY})

add_executable(tb-ryu-bench Ryu.bm.cpp)
target_link_libraries(tb-ryu-bench ${tb_bm_LIBRARY})

add_executable(tb-time-bench Time.bm.cpp)
target_link_libraries(tb-time-bench ${tb_bm_LIBRARY})

add_executable(tb-timer-bench Timer.bm.cpp)
target_link_libraries(tb-timer-bench ${tb_bm_LIBRARY})

add_executable(tb-util-bench Util.bm.cpp)
target_link_libraries(tb-util-bench ${tb_bm_LIBRARY})
