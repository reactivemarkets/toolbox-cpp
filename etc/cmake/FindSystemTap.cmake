# The Reactive C++ Toolbox.
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

# - Find SystemTap
# Find the native SystemTap headers.
#
#  SYSTEMTAP_INCLUDE_DIRS - Where to find sdt.h, etc.
#  SYSTEMTAP_FOUND        - True if SystemTap found.

# Look for the header file.
find_path(SYSTEMTAP_INCLUDE_DIR NAMES sys/sdt.h)

# Handle the QUIETLY and REQUIRED arguments and set SYSTEMTAP_FOUND to TRUE if all listed variables
# are TRUE.
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(SystemTap
        FOUND_VAR     SYSTEMTAP_FOUND
        REQUIRED_VARS SYSTEMTAP_INCLUDE_DIR
        FAIL_MESSAGE  "Failed to find SystemTap")

mark_as_advanced(SYSTEMTAP_INCLUDE_DIR)

# Copy the results to the output variables.
if(SYSTEMTAP_FOUND)
    set(SYSTEMTAP_INCLUDE_DIRS ${SYSTEMTAP_INCLUDE_DIR})
endif()
