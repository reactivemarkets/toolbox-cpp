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
  toolbox-map-bench
  toolbox-ryu-bench
  toolbox-time-bench
  toolbox-timer-bench
  toolbox-util-bench)

add_custom_target(toolbox-bench DEPENDS ${targets})

add_executable(toolbox-map-bench Map.bm.cpp)
target_link_libraries(toolbox-map-bench ${toolbox_bm_LIBRARY})

add_executable(toolbox-ryu-bench Ryu.bm.cpp)
target_link_libraries(toolbox-ryu-bench ${toolbox_bm_LIBRARY})

add_executable(toolbox-time-bench Time.bm.cpp)
target_link_libraries(toolbox-time-bench ${toolbox_bm_LIBRARY})

add_executable(toolbox-timer-bench Timer.bm.cpp)
target_link_libraries(toolbox-timer-bench ${toolbox_bm_LIBRARY})

add_executable(toolbox-util-bench Util.bm.cpp)
target_link_libraries(toolbox-util-bench ${toolbox_bm_LIBRARY})
