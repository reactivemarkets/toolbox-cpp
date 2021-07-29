# The Reactive C++ Toolbox.
# Copyright (C) 2013-2019 Swirly Cloud Limited
# Copyright (C) 2021 Reactive Markets Limited
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

if(POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif()

# Version 3.1 required for CMAKE_CXX_STANDARD.
cmake_minimum_required(VERSION 3.18)
project(toolbox-cpp)

set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${PROJECT_SOURCE_DIR}/etc/cmake")

include(CheckIPOSupported)
include(GNUInstallDirs)

# Configuration options:
set(REACTIVE_BUILD_ARCH "native" CACHE STRING "Target architecture.")
set(REACTIVE_BUILD_SHARED ON CACHE BOOL "Enable shared libs.")
set(REACTIVE_VERSION "snapshot" CACHE STRING "Release version.")
set(REACTIVE_TOOLSET "" CACHE PATH "Toolset prefix.")

# Toolbox options.
set(TOOLBOX_BUILD_ARCH   "${REACTIVE_BUILD_ARCH}")
set(TOOLBOX_BUILD_SHARED "${REACTIVE_BUILD_SHARED}")
set(TOOLBOX_VERSION      "${REACTIVE_VERSION}")
set(TOOLBOX_TOOLSET      "${REACTIVE_TOOLSET}")

if(NOT "${TOOLBOX_TOOLSET}" STREQUAL "")
  get_filename_component(TOOLBOX_TOOLSET "${TOOLBOX_TOOLSET}" REALPATH)
  set(CMAKE_PREFIX_PATH "${TOOLBOX_TOOLSET}")
  message(STATUS "TOOLBOX_TOOLSET: ${TOOLBOX_TOOLSET}")
endif()

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

# Address sanitizer build
set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fsanitize=address")
set(CMAKE_CXX_FLAGS_ASAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=address")

# Thread sanitizer build
set(CMAKE_C_FLAGS_TSAN "${CMAKE_C_FLAGS_RELWITHDEBINFO} -fsanitize=thread")
set(CMAKE_CXX_FLAGS_TSAN "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -fsanitize=thread")

if(NOT CMAKE_BUILD_TYPE)
  #set(CMAKE_BUILD_TYPE Debug)
  #set(CMAKE_BUILD_TYPE Release)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Choose the type of build: Debug, Release, RelWithDebInfo*, MinSizeRel Perf Coverage Asan Tsan" FORCE)
  #set(CMAKE_BUILD_TYPE MinSizeRel)
  #set(CMAKE_BUILD_TYPE Perf)
  #set(CMAKE_BUILD_TYPE Coverage)
  #set(CMAKE_BUILD_TYPE Asan)
  #set(CMAKE_BUILD_TYPE Tsan)
endif()

message(STATUS "Build architecture: ${TOOLBOX_BUILD_ARCH}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Build version: ${TOOLBOX_VERSION}")

# Optional IPO. Do not use IPO if it's not supported by compiler.
check_ipo_supported(RESULT ipo_supported OUTPUT ipo_supported_output)
if(ipo_supported)
  message(STATUS "IPO/LTO enabled")
  set(INTERPROCEDURAL_OPTIMIZATION TRUE)
else()
  message(WARNING "IPO/LTO not supported: ${ipo_supported_output}")
endif()

# Enable for use with clang-tidy.
if(NOT CMAKE_EXPORT_COMPILE_COMMANDS)
  set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(CMAKE_C_STANDARD 11)
# Emits -std=gnu++1z on OSX which is not working as expected.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

set(COMMON_WARN "-Wall -Werror -Wextra -Wstrict-aliasing=2 -Wno-unused-parameter -Wno-unused-variable")
set(COMMON_C_FLAGS "-fstrict-aliasing -fvisibility=hidden")
set(COMMON_CXX_FLAGS "${COMMON_C_FLAGS} -faligned-new")

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  set(COMMON_WARN "${COMMON_WARN} -Wno-implicit-fallthrough -Wno-psabi")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS} ${COMMON_WARN}")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} ${COMMON_WARN} -fno-enforce-eh-specs -fnothrow-opt")
elseif("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMMON_C_FLAGS} ${COMMON_WARN}")
  # Not all C++ attributes are supported by Clang.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMMON_CXX_FLAGS} ${COMMON_WARN} -Wno-unknown-attributes")
endif()
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS}")

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  set(TOOLBOX_BUILD_DEBUG 1)
else()
  set(TOOLBOX_BUILD_DEBUG 0)
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

set(Boost_REALPATH ON)

find_package(Boost 1.74 REQUIRED COMPONENTS date_time unit_test_framework)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
install_libraries("${Boost_DATE_TIME_LIBRARY_RELEASE}" "${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}")
message(STATUS "boost_date_time: ${Boost_DATE_TIME_LIBRARY_RELEASE}")
message(STATUS "boost_unit_test_framework: ${Boost_UNIT_TEST_FRAMEWORK_LIBRARY_RELEASE}")

add_definitions(-DBOOST_NO_AUTO_PTR=1 -DBOOST_ASIO_DISABLE_THREADS=1 -DBOOST_ASIO_HEADER_ONLY=1)
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
  set(tb_core_LIBRARY tb-core-shared)
  set(tb_bm_LIBRARY tb-bm-shared)
else()
  set(tb_core_LIBRARY tb-core-static)
  set(tb_bm_LIBRARY tb-bm-static)
endif()

install(FILES
  CODE_OF_CONDUCT.md
  CONTRIBUTING.md
  LICENSE.md
  README.md
  DESTINATION doc
  COMPONENT doc
)

# Enable cppcheck if program exists.
find_program(CPPCHECK NAMES cppcheck HINTS "${TOOLBOX_TOOLSET}/bin")
if(CPPCHECK)
  set(CMAKE_CXX_CPPCHECK
    "${CPPCHECK}"
    --error-exitcode=1
    --quiet
    --std=c++20
    --suppressions-list=${CMAKE_SOURCE_DIR}/etc/cppcheck-suppressions.txt
    -DBOOST_AUTO_TEST_SUITE
    -DSIZE_MAX=UINT64_MAX
  )
endif()

# Enable clang-tidy if program exists.
find_program(CLANG_TIDY clang-tidy HINTS "${TOOLBOX_TOOLSET}/bin")
if(CLANG_TIDY AND "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_CXX_CLANG_TIDY
    "${CLANG_TIDY}"
    "--header-filter=${CMAKE_SOURCE_DIR}/.*[.](hpp|hxx)$"
  )
  configure_file("${PROJECT_SOURCE_DIR}/.clang-tidy"
    "${CMAKE_BINARY_DIR}/.clang-tidy" @ONLY)
endif()

# Add clang-format target if tool is available.
find_program(CLANG_FORMAT clang-format HINTS "${TOOLBOX_TOOLSET}/bin")
if(CLANG_FORMAT)
  message(STATUS "Found clang-format")
  configure_file("${PROJECT_SOURCE_DIR}/clang-format.sh.in"
    "${CMAKE_BINARY_DIR}/clang-format.sh" @ONLY)
  add_custom_target(clang-format COMMAND bash "${CMAKE_BINARY_DIR}/clang-format.sh")
endif()

enable_testing()

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
