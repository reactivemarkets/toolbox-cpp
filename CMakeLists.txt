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

# Version 3.1 required for CMAKE_CXX_STANDARD.
cmake_minimum_required(VERSION 3.12)
project(toolbox-cpp)
include(GNUInstallDirs)

# Configuration options:
set(TOOLBOX_BUILD_ARCH "native" CACHE STRING "Target architecture.")
set(TOOLBOX_BUILD_SHARED ON CACHE BOOL "Enable shared libs.")
set(TOOLBOX_VERSION "snapshot" CACHE STRING "Release version.")
set(TOOLBOX_TOOLCHAIN "" CACHE PATH "Toolchain prefix.")

if (NOT "${TOOLBOX_TOOLCHAIN}" STREQUAL "")
  get_filename_component(TOOLBOX_TOOLCHAIN "${TOOLBOX_TOOLCHAIN}" REALPATH)
  set(CMAKE_PREFIX_PATH "${TOOLBOX_TOOLCHAIN}")
  message(STATUS "TOOLBOX_TOOLCHAIN: ${TOOLBOX_TOOLCHAIN}")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/etc/cmake")

# Optimise for target architecture.
set(CMAKE_C_FLAGS_RELEASE          "${CMAKE_C_FLAGS_RELEASE} -march=${TOOLBOX_BUILD_ARCH}")
set(CMAKE_CXX_FLAGS_RELEASE        "${CMAKE_CXX_FLAGS_RELEASE} -march=${TOOLBOX_BUILD_ARCH}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO   "${CMAKE_C_FLAGS_RELWITHDEBINFO} -march=${TOOLBOX_BUILD_ARCH}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -march=${TOOLBOX_BUILD_ARCH}")
set(CMAKE_C_FLAGS_MINSIZEREL       "${CMAKE_C_FLAGS_MINSIZEREL} -march=${TOOLBOX_BUILD_ARCH}")
set(CMAKE_CXX_FLAGS_MINSIZEREL     "${CMAKE_CXX_FLAGS_MINSIZEREL} -march=${TOOLBOX_BUILD_ARCH}")

set(CMAKE_C_FLAGS_DEBUG   "${CMAKE_C_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

# Intended for use with Linux Perf.
set(CMAKE_C_FLAGS_PERF   "${CMAKE_C_FLAGS_RELEASE} -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_PERF "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer")

# Coverage build
set(CMAKE_C_FLAGS_COVERAGE "${CMAKE_C_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")
set(CMAKE_CXX_FLAGS_COVERAGE "${CMAKE_CXX_FLAGS_DEBUG} -fprofile-arcs -ftest-coverage")


if(NOT CMAKE_BUILD_TYPE)
  #set(CMAKE_BUILD_TYPE DEBUG)
  #set(CMAKE_BUILD_TYPE RELEASE)
  set(CMAKE_BUILD_TYPE RELWITHDEBINFO CACHE STRING "Choose the type of build : DEBUG, RELEASE, RELWITHDEBINFO*, MINSIZEREL PERF COVERAGE" FORCE)
  #set(CMAKE_BUILD_TYPE MINSIZEREL)
  #set(CMAKE_BUILD_TYPE PERF)
  #set(CMAKE_BUILD_TYPE COVERAGE)
endif()

message(STATUS "Build architecture: ${TOOLBOX_BUILD_ARCH}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build version: ${TOOLBOX_VERSION}")

# Enable for use with clang-tidy.
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

string(TOUPPER "${CMAKE_BUILD_TYPE}" CMAKE_BUILD_TYPE)

set(CMAKE_C_STANDARD 11)
# Emits -std=gnu++1z on OSX which is not working as expected.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_WARN "-Wall -Werror -Wextra -Wstrict-aliasing=2 -Wno-unused-parameter -Wno-unused-variable")
set(COMMON_C_FLAGS "-fstrict-aliasing -fvisibility=hidden")
set(COMMON_CXX_FLAGS "${COMMON_C_FLAGS} -faligned-new -fno-rtti")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(COMMON_WARN "${COMMON_WARN} -Wno-implicit-fallthrough -Wno-psabi")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS} ${COMMON_WARN}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} ${COMMON_WARN} -fno-enforce-eh-specs -fnothrow-opt")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS} ${COMMON_WARN}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} ${COMMON_WARN}")
endif()
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL "DEBUG")
  add_definitions(-DTOOLBOX_BUILD_DEBUG=1)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH ON)
set(CMAKE_INSTALL_RPATH "$ORIGIN/../${CMAKE_INSTALL_LIBDIR}")

find_program(DOT_EXECUTABLE dot)

function(install_libraries)
  foreach(lib ${ARGN})
    if(NOT "${lib}" MATCHES "\\.a$")
      get_filename_component(dir "${lib}" DIRECTORY)
      get_filename_component(name "${lib}" NAME_WE)
      # Glob library and symlinks.
      file(GLOB files "${dir}/${name}.*")
      foreach (file ${files})
        if(NOT "${file}" MATCHES "\\.a$")
          message(STATUS "Runtime dependency: ${file}")
          install(FILES "${file}" DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT shared)
        endif()
      endforeach()
    endif()
  endforeach()
endfunction()

include_directories(
  "${PROJECT_SOURCE_DIR}"
  "${CMAKE_BINARY_DIR}/include")

if(NOT TOOLBOX_BUILD_SHARED)
  set(Boost_USE_STATIC_LIBS ON)
endif()
set(Boost_REALPATH ON)

find_package(Boost 1.67 REQUIRED COMPONENTS date_time unit_test_framework)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
install_libraries("${Boost_DATE_TIME_LIBRARY_RELEASE}" "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}")
message(STATUS "boost_date_time: ${Boost_DATE_TIME_LIBRARY_RELEASE}")
message(STATUS "boost_unit_test_framework: ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}")

add_definitions(-DBOOST_NO_AUTO_PTR=1 -DBOOST_NO_RTTI=1 -DBOOST_NO_TYPEID=1)
add_definitions(-DBOOST_ASIO_DISABLE_THREADS=1 -DBOOST_ASIO_HEADER_ONLY=1)
if(NOT "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}" MATCHES "\\.a$")
  add_definitions(-DBOOST_TEST_DYN_LINK)
endif()

find_package(SystemTap)
if(SYSTEMTAP_FOUND)
  set(TOOLBOX_HAVE_SYSTEMTAP 1)
else()
  set(TOOLBOX_HAVE_SYSTEMTAP 0)
endif()

find_package(Doxygen) # Optional.

if(TOOLBOX_BUILD_SHARED)
  set(toolbox_LIBRARY toolbox-shared)
  set(toolbox_bm_LIBRARY toolbox-bm-shared)
else()
  set(toolbox_LIBRARY toolbox-static)
  set(toolbox_bm_LIBRARY toolbox-bm-static)
endif()

enable_testing()

# Add cppcheck target if tool is available.
find_program(CPPCHECK cppcheck HINTS "${TOOLBOX_TOOLCHAIN}/bin")
if (CPPCHECK)
  message(STATUS "Found cppcheck")
  cmake_host_system_information(RESULT CORE_COUNT QUERY NUMBER_OF_LOGICAL_CORES)
  configure_file("${PROJECT_SOURCE_DIR}/cppcheck.sh.in"
    "${CMAKE_BINARY_DIR}/cppcheck.sh" @ONLY)
  add_custom_target(cppcheck COMMAND bash "${CMAKE_BINARY_DIR}/cppcheck.sh")
endif()

# Add clang-format target if tool is available.
find_program(CLANG_FORMAT clang-format HINTS "${TOOLBOX_TOOLCHAIN}/bin")
if (CLANG_FORMAT)
  message(STATUS "Found clang-format")
  configure_file("${PROJECT_SOURCE_DIR}/clang-format.sh.in"
    "${CMAKE_BINARY_DIR}/clang-format.sh" @ONLY)
  add_custom_target(clang-format COMMAND bash "${CMAKE_BINARY_DIR}/clang-format.sh")
endif()

# Add clang-tidy target if tool is available.
find_program(CLANG_TIDY run-clang-tidy.py HINTS "/usr/share/clang/"
  "${TOOLBOX_TOOLCHAIN}/share/clang")
if (CLANG_TIDY)
  message(STATUS "Found clang-tidy")
  cmake_host_system_information(RESULT CORE_COUNT QUERY NUMBER_OF_LOGICAL_CORES)
  add_custom_target(clang-tidy COMMAND "${CLANG_TIDY}"
        -quiet
        -j ${CORE_COUNT}
        -p "${CMAKE_BINARY_DIR}")
endif()

install(FILES
  CODE_OF_CONDUCT.md
  CONTRIBUTING.md
  LICENSE.md
  README.md
  DESTINATION doc
  COMPONENT doc
)

add_subdirectory(toolbox)
add_subdirectory(bench)
add_subdirectory(example)
add_subdirectory(image)
add_subdirectory(doc)
add_subdirectory(etc)

set(CPACK_GENERATOR "TGZ;RPM;DEB")

set(CPACK_OUTPUT_FILE_PREFIX dist)
set(CPACK_PACKAGE_CONTACT "development@reactivemarkets.com")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Toolbox - The Reactive C++ Toolbox")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/reactivemarkets/toolbox-cpp")
set(CPACK_PACKAGE_NAME ${CMAKE_PROJECT_NAME})
set(CPACK_PACKAGE_VENDOR "Reactive Markets Limited")
set(CPACK_PACKAGE_VERSION "${TOOLBOX_VERSION}")
set(CPACK_PACKAGING_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
set(CPACK_SYSTEM_NAME ${CMAKE_SYSTEM_PROCESSOR})

set(CPACK_ALL_INSTALL_TYPES Standard Developer)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL doc config header program shared static)
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)

# Runtime.

set(CPACK_COMPONENT_CONFIG_DISPLAY_NAME "Configuration")
set(CPACK_COMPONENT_CONFIG_GROUP "Runtime")
set(CPACK_COMPONENT_CONFIG_INSTALL_TYPES Standard Developer)

set(CPACK_COMPONENT_PROGRAM_DISPLAY_NAME "Programs")
set(CPACK_COMPONENT_PROGRAM_GROUP "Runtime")
set(CPACK_COMPONENT_PROGRAM_INSTALL_TYPES Standard Developer)

set(CPACK_COMPONENT_SHARED_DISPLAY_NAME "Shared Libs")
set(CPACK_COMPONENT_SHARED_GROUP "Runtime")
set(CPACK_COMPONENT_SHARED_INSTALL_TYPES Standard Developer)

# Development.

set(CPACK_COMPONENT_DOC_DISPLAY_NAME "Documentation")
set(CPACK_COMPONENT_DOC_GROUP "Development")
set(CPACK_COMPONENT_DOC_INSTALL_TYPES Standard Developer)

set(CPACK_COMPONENT_HEADER_DISPLAY_NAME "C++ Headers")
set(CPACK_COMPONENT_HEADER_GROUP "Development")
set(CPACK_COMPONENT_HEADER_INSTALL_TYPES Developer)
set(CPACK_COMPONENT_HEADER_DEPENDS static)

set(CPACK_COMPONENT_STATIC_DISPLAY_NAME "Static Libs")
set(CPACK_COMPONENT_STATIC_GROUP "Development")
set(CPACK_COMPONENT_STATIC_INSTALL_TYPES Developer)
set(CPACK_COMPONENT_STATIC_DEPENDS header)

set(CPACK_PROJECT_CONFIG_FILE "${PROJECT_SOURCE_DIR}/CPackProjectConfig.cmake")
include(CPack)
