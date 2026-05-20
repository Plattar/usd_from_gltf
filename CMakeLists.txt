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

cmake_minimum_required(VERSION 3.16)
project(usd_from_gltf LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# PIC is required for the import plugin (shared lib) and harmless elsewhere.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ----- Locate OpenUSD -----
#
# USD 26.05 ships a CMake package config at <prefix>/pxrConfig.cmake. Point
# USD_DIR at the install prefix (e.g. -DUSD_DIR=/opt/openusd-26.05) or set
# CMAKE_PREFIX_PATH directly.
if (DEFINED USD_DIR AND NOT pxr_DIR)
  set(pxr_DIR "${USD_DIR}" CACHE PATH "USD install prefix" FORCE)
  list(APPEND CMAKE_PREFIX_PATH "${USD_DIR}")
endif ()
find_package(pxr CONFIG REQUIRED)

# RPATH so the binaries can find USD's shared libs on Linux/OSX.
if (APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@loader_path/../lib" "${PXR_CMAKE_DIR}/lib")
elseif (UNIX)
  list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib" "${PXR_CMAKE_DIR}/lib")
endif ()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# The USD libraries needed across this project. We define an interface target
# so subdirectories can simply link `usd_from_gltf::usd_libs`.
add_library(ufg_usd_libs INTERFACE)
add_library(usd_from_gltf::usd_libs ALIAS ufg_usd_libs)
target_link_libraries(ufg_usd_libs INTERFACE
  arch
  gf
  plug
  sdf
  tf
  usd
  usdGeom
  usdShade
  usdSkel
  usdUtils
  vt
)

# ----- Platform settings -----
if (MSVC)
  add_compile_definitions(
    _CRT_SECURE_NO_WARNINGS
    NOMINMAX
  )
  add_compile_options(
    /wd4996 # 'std::copy' marked as unsafe
    /wd4003 # not enough actual parameters for macro
    /wd4244 # possible loss of data
    /wd4267 # possible loss of data
    /wd4305 # truncation 'double' -> 'float'
  )
  # Force release CRT so we can build this lib in Debug without rebuilding deps.
  foreach (flag
    CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
    CMAKE_C_FLAGS   CMAKE_C_FLAGS_DEBUG   CMAKE_C_FLAGS_RELEASE
  )
    string(REPLACE "/MDd" "/MD" ${flag} "${${flag}}")
    string(REPLACE "/RTC1" "" ${flag} "${${flag}}")
  endforeach ()
else ()
  add_compile_options(
    -Wno-deprecated # Silence deprecation warnings from USD headers.
  )
endif ()

add_subdirectory(common)
add_subdirectory(convert)
add_subdirectory(gltf)
add_subdirectory(process)
add_subdirectory(usd_from_gltf)
add_subdirectory(ufg_plugin)

install(EXPORT ufglib DESTINATION lib/ufg)
install(FILES ufg-config.cmake DESTINATION .)
