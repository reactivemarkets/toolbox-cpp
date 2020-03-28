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

install(FILES
  Algorithms.md
  BuildFromSource.md
  Concurrency.md
  DataStructures.md
  Dependencies.md
  Messaging.md
  MoSCoW.md
  StringHandling.md
  DESTINATION doc
  COMPONENT doc
)

set(doc_DEPENDS)

if(DOXYGEN_FOUND)
  configure_file(Doxyfile.in "${PROJECT_BINARY_DIR}/doc/Doxyfile" @ONLY)

  add_custom_target(tb-doxygen
    WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    COMMAND "${DOXYGEN_EXECUTABLE}" "${PROJECT_BINARY_DIR}/doc/Doxyfile"
    COMMAND "${CMAKE_COMMAND}" -E echo docs.toolbox.reactivemarkets.com
           >"${PROJECT_BINARY_DIR}/doc/html/CNAME"
    SOURCES "${PROJECT_BINARY_DIR}/doc/Doxyfile")

  add_dependencies(tb-doxygen tb-image)
  set(doc_DEPENDS ${doc_DEPENDS} tb-doxygen)
endif()

add_custom_target(tb-doc DEPENDS ${doc_DEPENDS})
