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
  tb-echo-clnt
  tb-echo-serv
  tb-http-serv)

add_custom_target(tb-example DEPENDS ${targets})

add_executable(tb-echo-clnt EchoClnt.cpp)
target_link_libraries(tb-echo-clnt ${tb_core_LIBRARY})

add_executable(tb-echo-serv EchoServ.cpp)
target_link_libraries(tb-echo-serv ${tb_core_LIBRARY})

add_executable(tb-http-serv HttpServ.cpp)
target_link_libraries(tb-http-serv ${tb_core_LIBRARY})
