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

include(AddFileDependencies)

set(outfiles)
if(DOT_EXECUTABLE)

  message(STATUS "Dot found: ${DOT_EXECUTABLE}")

  set(names
    util
    sys
    io
    ipc
    net
    http
  )

  foreach(name ${names})

    set(outfile "${CMAKE_BINARY_DIR}/image/tb-${name}.png")

    add_custom_command(
      OUTPUT "${outfile}"
      WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
      COMMAND CXX="${CMAKE_CXX_COMPILER}" "${PERL}" etc/dotdepend.pl ${name}
              | "${DOT_EXECUTABLE}" -Tpng >"${outfile}"
      DEPENDS "${PROJECT_SOURCE_DIR}/etc/dotdepend.pl")

    set(outfiles ${outfiles} "${outfile}")
    install(FILES "${outfile}" DESTINATION image COMPONENT doc)
  endforeach()

endif()

add_custom_target(tb-image ALL DEPENDS ${outfiles})
