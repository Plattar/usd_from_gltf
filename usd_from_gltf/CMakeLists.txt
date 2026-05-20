# Copyright 2019 Google LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# TCLAP is header-only. Most package managers ship a CMake config; fall back to
# a TCLAP_INCLUDE_DIR cache var when not available.
find_package(TCLAP CONFIG QUIET)
if (NOT TCLAP_FOUND)
  find_path(TCLAP_INCLUDE_DIR NAMES tclap/CmdLine.h)
  if (NOT TCLAP_INCLUDE_DIR)
    message(FATAL_ERROR
      "TCLAP not found. Install it (brew install tclap, apt install libtclap-dev) "
      "or set -DTCLAP_INCLUDE_DIR=<dir containing tclap/CmdLine.h>.")
  endif ()
endif ()

add_executable(usd_from_gltf
  args.cc
  args.h
  usd_from_gltf.cc
)

target_link_libraries(usd_from_gltf
  PRIVATE convert usd_from_gltf::usd_libs
)

if (TCLAP_FOUND)
  target_link_libraries(usd_from_gltf PRIVATE TCLAP::TCLAP)
else ()
  target_include_directories(usd_from_gltf PRIVATE "${TCLAP_INCLUDE_DIR}")
endif ()

install(TARGETS usd_from_gltf DESTINATION bin)
