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
# Point USD_DIR at the OpenUSD install prefix (e.g. -DUSD_DIR=/opt/openusd-26.05).
# Two layouts are supported:
#
#   1. "Full" install — contains pxrConfig.cmake. We use find_package(pxr CONFIG)
#      which sets up the proper imported targets with their transitive deps.
#
#   2. "Slim" install — pxrConfig.cmake / cmake/ have been stripped (some
#      container images do this to save space). We fall back to manually
#      adding ${USD_DIR}/include and finding the libraries via find_library.
if (NOT DEFINED USD_DIR AND DEFINED ENV{USD_BUILD_PATH})
  set(USD_DIR "$ENV{USD_BUILD_PATH}" CACHE PATH "OpenUSD install prefix")
endif ()

set(_ufg_pxr_config "${USD_DIR}/pxrConfig.cmake")
set(_ufg_pxr_config_lower "${USD_DIR}/cmake/pxrConfig.cmake")
if (USD_DIR AND (EXISTS "${_ufg_pxr_config}" OR EXISTS "${_ufg_pxr_config_lower}"))
  set(UFG_USD_VIA_PACKAGE ON)
  set(pxr_DIR "${USD_DIR}" CACHE PATH "USD pxrConfig.cmake dir" FORCE)
  list(APPEND CMAKE_PREFIX_PATH "${USD_DIR}")
  find_package(pxr CONFIG REQUIRED)
  message(STATUS "OpenUSD: using pxrConfig.cmake at ${USD_DIR}")
elseif (USD_DIR AND IS_DIRECTORY "${USD_DIR}/include/pxr" AND IS_DIRECTORY "${USD_DIR}/lib")
  set(UFG_USD_VIA_PACKAGE OFF)
  message(STATUS "OpenUSD: pxrConfig.cmake not present at ${USD_DIR}; "
                 "using fallback include/lib discovery.")
else ()
  message(FATAL_ERROR
    "OpenUSD install not found. Set -DUSD_DIR=<prefix> to a directory that "
    "contains either pxrConfig.cmake, or include/pxr/ and lib/ subdirectories.")
endif ()

set(UFG_USD_LIB_DIR "${USD_DIR}/lib")

# RPATH so the binaries can find USD's shared libs on Linux/OSX.
if (APPLE)
  list(APPEND CMAKE_INSTALL_RPATH "@loader_path/../lib" "${UFG_USD_LIB_DIR}")
elseif (UNIX)
  list(APPEND CMAKE_INSTALL_RPATH "$ORIGIN/../lib" "${UFG_USD_LIB_DIR}")
endif ()
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_RPATH_USE_ORIGIN TRUE)
list(APPEND CMAKE_BUILD_RPATH "${UFG_USD_LIB_DIR}")

# Interface target that bundles every USD library this project needs. Subdirs
# only have to link `usd_from_gltf::usd_libs`.
add_library(ufg_usd_libs INTERFACE)
add_library(usd_from_gltf::usd_libs ALIAS ufg_usd_libs)

set(_ufg_usd_libnames
  arch gf plug sdf tf usd usdGeom usdShade usdSkel usdUtils vt
)

if (UFG_USD_VIA_PACKAGE)
  # pxrConfig.cmake registers each library as an imported target with the
  # short name (e.g. `usd`, `usdGeom`) — link them directly.
  target_link_libraries(ufg_usd_libs INTERFACE ${_ufg_usd_libnames})
else ()
  # Slim install: probe lib/ for the actual filenames. OpenUSD ≥ 24.05 prefixes
  # its shared libs with `libusd_` on Linux/macOS and `usd_` on Windows; older
  # versions used the bare name (`libgf.so`, `gf.lib`). We search for both.
  target_include_directories(ufg_usd_libs INTERFACE "${USD_DIR}/include")
  foreach (libname IN LISTS _ufg_usd_libnames)
    find_library(_ufg_lib_${libname}
      NAMES "usd_${libname}" "${libname}"
      HINTS "${UFG_USD_LIB_DIR}"
      NO_DEFAULT_PATH
    )
    if (NOT _ufg_lib_${libname})
      message(FATAL_ERROR
        "Could not locate OpenUSD library '${libname}' (tried 'usd_${libname}' "
        "and '${libname}') under ${UFG_USD_LIB_DIR}.")
    endif ()
    target_link_libraries(ufg_usd_libs INTERFACE "${_ufg_lib_${libname}}")
  endforeach ()

  # OpenUSD's headers transitively include TBB headers; depending on the build
  # we may also need to link tbb explicitly. build_usd.py installs tbb into
  # ${USD_DIR}/lib alongside the USD libs.
  find_library(_ufg_lib_tbb
    NAMES tbb tbb12
    HINTS "${UFG_USD_LIB_DIR}"
    NO_DEFAULT_PATH
  )
  if (_ufg_lib_tbb)
    target_link_libraries(ufg_usd_libs INTERFACE "${_ufg_lib_tbb}")
  endif ()

  if (UNIX)
    find_package(Threads REQUIRED)
    target_link_libraries(ufg_usd_libs INTERFACE Threads::Threads ${CMAKE_DL_LIBS})
  endif ()

  # OpenUSD ≥ 24.05 ships vendored boost::python under pxr/external/boost/python/.
  # Its detail headers (pulled in via tf/pySafePython.h → vt/value.h → most of
  # the public USD API) unconditionally #include <pyconfig.h> when USD was built
  # with PXR_PYTHON_SUPPORT_ENABLED. We only need the Python include path —
  # libpython itself is resolved transitively through the USD .so files.
  find_package(Python3 COMPONENTS Interpreter Development QUIET)
  if (NOT Python3_INCLUDE_DIRS AND Python3_Interpreter_FOUND)
    # Some slim images ship Python.h but no libpython, which makes the
    # "Development" component report NOTFOUND even when headers are usable.
    # Fall back to asking the python interpreter directly.
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c
        "import sysconfig; print(sysconfig.get_path('include'))"
      OUTPUT_VARIABLE _ufg_py_include
      OUTPUT_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE _ufg_py_rc
    )
    if (_ufg_py_rc EQUAL 0 AND EXISTS "${_ufg_py_include}/pyconfig.h")
      set(Python3_INCLUDE_DIRS "${_ufg_py_include}")
    endif ()
  endif ()
  if (NOT Python3_LIBRARIES AND Python3_Interpreter_FOUND)
    # Find libpython via sysconfig — Python3::Python may not be defined if the
    # Development component reported NOTFOUND.
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c
        "import sysconfig; print(sysconfig.get_config_var('LIBDIR'))"
      OUTPUT_VARIABLE _ufg_py_libdir
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
      COMMAND "${Python3_EXECUTABLE}" -c
        "import sysconfig; print(sysconfig.get_config_var('LDLIBRARY'))"
      OUTPUT_VARIABLE _ufg_py_ldlibrary
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if (_ufg_py_libdir AND _ufg_py_ldlibrary
        AND EXISTS "${_ufg_py_libdir}/${_ufg_py_ldlibrary}")
      set(Python3_LIBRARIES "${_ufg_py_libdir}/${_ufg_py_ldlibrary}")
    endif ()
  endif ()
  if (Python3_INCLUDE_DIRS)
    target_include_directories(ufg_usd_libs INTERFACE ${Python3_INCLUDE_DIRS})
    message(STATUS
      "OpenUSD: added Python include dirs (${Python3_INCLUDE_DIRS}) for USD headers.")
  else ()
    message(WARNING
      "Python development headers not found. If your OpenUSD install was built "
      "with PXR_PYTHON_SUPPORT_ENABLED, the build will fail on <pyconfig.h>. "
      "Install python3-dev or pass -DPython3_ROOT_DIR=<prefix>.")
  endif ()
  if (Python3_LIBRARIES)
    # USD's headers materialize references to libpython symbols (TfPyLock etc.)
    # in our translation units. With the linker's default --no-add-needed
    # policy, libpython must appear on the link line even though USD's .so
    # transitively depends on it.
    target_link_libraries(ufg_usd_libs INTERFACE ${Python3_LIBRARIES})
    message(STATUS "OpenUSD: linking libpython (${Python3_LIBRARIES}).")
  endif ()
endif ()

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
