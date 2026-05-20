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

# Locate image and mesh decoding libraries via the system / vcpkg / brew /
# build-system-installed CMake configs. Hint vars (STB_IMAGE_INCLUDE_DIR,
# DRACO_DIR, GIFLIB_INCLUDE_DIR, GIFLIB_LIBRARY) can override.

find_package(PNG REQUIRED)
find_package(JPEG QUIET) # libjpeg
find_package(ZLIB REQUIRED)

# libjpeg-turbo: prefer the TurboJPEG C API (turbojpeg.h). Most distros ship a
# pkg-config file 'libturbojpeg' or a CMake export.
find_library(TURBOJPEG_LIBRARY NAMES turbojpeg libturbojpeg)
find_path(TURBOJPEG_INCLUDE_DIR NAMES turbojpeg.h)
if (NOT TURBOJPEG_LIBRARY OR NOT TURBOJPEG_INCLUDE_DIR)
  message(FATAL_ERROR
    "libjpeg-turbo (TurboJPEG) not found. Install it (brew install jpeg-turbo, "
    "apt install libturbojpeg0-dev) or set TURBOJPEG_LIBRARY/TURBOJPEG_INCLUDE_DIR.")
endif ()

# giflib: header-only-ish C library, no standard CMake export.
find_library(GIFLIB_LIBRARY NAMES gif libgif)
find_path(GIFLIB_INCLUDE_DIR NAMES gif_lib.h)
if (NOT GIFLIB_LIBRARY OR NOT GIFLIB_INCLUDE_DIR)
  message(FATAL_ERROR
    "giflib not found. Install it (brew install giflib, apt install libgif-dev) "
    "or set GIFLIB_LIBRARY/GIFLIB_INCLUDE_DIR.")
endif ()

# stb_image: single-header. Some distros ship a stb pkg.
find_path(STB_IMAGE_INCLUDE_DIR NAMES stb_image.h PATH_SUFFIXES stb)
if (NOT STB_IMAGE_INCLUDE_DIR)
  message(FATAL_ERROR
    "stb_image.h not found. Install the 'stb' package or set "
    "STB_IMAGE_INCLUDE_DIR to the directory containing stb_image.h.")
endif ()

# Draco mesh compression. Upstream ships draco-config.cmake from ~1.3.6+.
find_package(draco CONFIG QUIET)
if (NOT draco_FOUND)
  find_library(DRACO_LIBRARY NAMES draco)
  find_path(DRACO_INCLUDE_DIR NAMES draco/compression/decode.h)
  if (NOT DRACO_LIBRARY OR NOT DRACO_INCLUDE_DIR)
    message(FATAL_ERROR
      "Draco not found. Install via brew (draco) / vcpkg / source build, "
      "or set DRACO_LIBRARY/DRACO_INCLUDE_DIR.")
  endif ()
endif ()

add_library(process
  image_png.h
  math.cc
  math.h
  mesh.cc
  mesh.h
  process_util.cc
  process_util.h
  skin.cc
  skin.h
  access.h
  animation.cc
  animation.h
  color.cc
  color.h
  float_image.cc
  float_image.h
  image.cc
  image.h
  image_fallback.cc
  image_fallback.h
  image_gif.cc
  image_gif.h
  image_jpg.cc
  image_jpg.h
  image_png.cc
)

target_include_directories(process
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include/ufg>
  PRIVATE
    "${STB_IMAGE_INCLUDE_DIR}"
    "${GIFLIB_INCLUDE_DIR}"
    "${TURBOJPEG_INCLUDE_DIR}"
)

target_link_libraries(process
  PUBLIC common
  PRIVATE
    PNG::PNG
    ZLIB::ZLIB
    "${TURBOJPEG_LIBRARY}"
    "${GIFLIB_LIBRARY}"
)

if (draco_FOUND)
  target_link_libraries(process PRIVATE draco::draco)
else ()
  target_include_directories(process PRIVATE "${DRACO_INCLUDE_DIR}")
  target_link_libraries(process PRIVATE "${DRACO_LIBRARY}")
endif ()

file(GLOB PROCESS_HEADERS "*.h")
set_target_properties(process PROPERTIES PUBLIC_HEADER "${PROCESS_HEADERS}")

install(TARGETS process
  ARCHIVE DESTINATION lib/ufg
  LIBRARY DESTINATION lib/ufg
  PUBLIC_HEADER DESTINATION include/ufg/process
)
