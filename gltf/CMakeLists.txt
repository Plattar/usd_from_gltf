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

# nlohmann/json: header-only. Found via its CMake package on most distros.
# Fall back to a JSON_INCLUDE_DIR cache var if the package is not installed.
find_package(nlohmann_json CONFIG QUIET)
if (NOT nlohmann_json_FOUND)
  if (NOT DEFINED JSON_INCLUDE_DIR)
    message(FATAL_ERROR
      "nlohmann_json not found via find_package. Install it (brew/apt/vcpkg) "
      "or set -DJSON_INCLUDE_DIR=<dir containing nlohmann/json.hpp>.")
  endif ()
endif ()

add_library(gltf
  internal_util.cc
  internal_util.h
  load.cc
  load.h
  memory_stream.cc
  memory_stream.h
  message.cc
  message.h
  messages.inl
  stream.cc
  stream.h
  validate.cc
  validate.h
  cache.cc
  cache.h
  disk_stream.cc
  disk_stream.h
  disk_util.cc
  disk_util.h
  glb_stream.cc
  glb_stream.h
  gltf.cc
  gltf.h
  image_parsing.cc
  image_parsing.h
)

target_include_directories(gltf PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
  $<INSTALL_INTERFACE:include/ufg>
)

if (nlohmann_json_FOUND)
  target_link_libraries(gltf PRIVATE nlohmann_json::nlohmann_json)
else ()
  target_include_directories(gltf PRIVATE "${JSON_INCLUDE_DIR}")
endif ()

file(GLOB GLTF_HEADERS "*.h")
set_target_properties(gltf PROPERTIES PUBLIC_HEADER "${GLTF_HEADERS}")

install(TARGETS gltf
  ARCHIVE DESTINATION lib/ufg
  LIBRARY DESTINATION lib/ufg
  PUBLIC_HEADER DESTINATION include/ufg/gltf
)
