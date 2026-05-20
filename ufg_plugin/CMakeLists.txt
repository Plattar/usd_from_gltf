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

add_library(ufg_plugin SHARED
  ufg_plugin.cc
  ufg_plugin.h
)

set_target_properties(ufg_plugin PROPERTIES PREFIX "")

target_link_libraries(ufg_plugin
  PRIVATE convert usd_from_gltf::usd_libs
)

# Resolve the runtime filename USD's plugin loader should reference.
if (WIN32)
  set(UFG_PLUGIN_LIBRARY_FILENAME "ufg_plugin.dll")
elseif (APPLE)
  set(UFG_PLUGIN_LIBRARY_FILENAME "ufg_plugin.dylib")
else ()
  set(UFG_PLUGIN_LIBRARY_FILENAME "ufg_plugin.so")
endif ()

configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/plugInfo.json.in"
  "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
  @ONLY
)

# Install layout matches USD plugin convention: <plugin_dir>/<name>/resources/plugInfo.json
install(TARGETS ufg_plugin
  LIBRARY DESTINATION plugin/usd/ufg_plugin
  RUNTIME DESTINATION plugin/usd/ufg_plugin
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/plugInfo.json"
  DESTINATION plugin/usd/ufg_plugin/resources
)
