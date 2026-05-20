# USD from glTF

Library, command-line tool, and import plugin for converting [glTF](https://www.khronos.org/gltf/) models to [USD](https://openusd.org/) formatted assets for display in [AR Quick Look](https://developer.apple.com/arkit/gallery).

*This fork updates the original Google project to build against modern OpenUSD (tested against the API surface of OpenUSD 26.05). It is not an officially supported Google or Pixar product.*

This is a C++ native library that serves as an alternative to existing scripted solutions. Its main benefits are improved compatibility with iOS and conversion speed (see [Compatibility](#compatibility) and [Performance](#performance)). It treats USDZ as a transmission format rather than an interchange format. For more information about transmission and interchange file formats see [here](http://nickporcino.com/posts/last_mile_interchange.html).

TLDR: [Build](#building) it, then [convert](#using-the-command-line-tool) with: `usd_from_gltf <source.gltf> <destination.usdz>`

## Background
glTF is a transmission format for 3D assets that is well suited to the web and mobile devices by removing data that is not important for efficient display of assets. USD is an interchange format that can be used for file editing in Digital Content Creation tools (ie. Maya).

However, iOS Quick Look supports displaying [USDZ](https://openusd.org/release/spec_usdz.html) files with a subset of the USD file specification. This tool converts glTF files to USDZ for display in Quick Look, attempting to emulate as much of glTF's functionality as possible in iOS Quick Look runtime.

The emulation process is lossy. For example, to support double sided glTF materials, the geometry is doubled. This allows the converted glTF to display correctly on iOS, but importing back into a DCC application will not be the same data as the original source file.

## Building

### Prerequisites

You need a working OpenUSD install (≥ 24.05 should work, tested against 26.05's API
surface) and a handful of common image / mesh libraries:

| Dependency       | macOS (Homebrew)             | Debian/Ubuntu                                | Notes                            |
| ---------------- | ---------------------------- | -------------------------------------------- | -------------------------------- |
| OpenUSD          | build from source            | build from source                            | provides `pxrConfig.cmake`       |
| CMake ≥ 3.16     | `brew install cmake`         | `apt install cmake`                          |                                  |
| nlohmann_json    | `brew install nlohmann-json` | `apt install nlohmann-json3-dev`             | header-only                      |
| TCLAP            | `brew install tclap`         | `apt install libtclap-dev`                   | header-only                      |
| libpng           | `brew install libpng`        | `apt install libpng-dev`                     |                                  |
| libjpeg-turbo    | `brew install jpeg-turbo`    | `apt install libturbojpeg0-dev`              | provides `turbojpeg.h`           |
| giflib           | `brew install giflib`        | `apt install libgif-dev`                     |                                  |
| zlib             | (system)                     | `apt install zlib1g-dev`                     |                                  |
| Draco            | `brew install draco`         | install from source / vcpkg                  | mesh decompression               |
| stb_image        | `brew install stb`           | install from source (it's a single header)   | `stb_image.h` only               |

### Configure & build

```sh
cmake -S . -B build \
  -DCMAKE_BUILD_TYPE=Release \
  -DUSD_DIR=/path/to/openusd-26.05    # install prefix containing pxrConfig.cmake
cmake --build build --parallel
cmake --install build --prefix /path/to/ufg
```

If any of the optional dependencies don't auto-resolve, point CMake at them
explicitly, e.g.:

```sh
-DJSON_INCLUDE_DIR=/opt/include          # directory containing nlohmann/json.hpp
-DTCLAP_INCLUDE_DIR=/opt/include         # directory containing tclap/CmdLine.h
-DSTB_IMAGE_INCLUDE_DIR=/opt/include     # directory containing stb_image.h
-DTURBOJPEG_LIBRARY=/opt/lib/libturbojpeg.so
-DTURBOJPEG_INCLUDE_DIR=/opt/include
-DGIFLIB_LIBRARY=/opt/lib/libgif.so
-DGIFLIB_INCLUDE_DIR=/opt/include
-DDRACO_LIBRARY=/opt/lib/libdraco.a
-DDRACO_INCLUDE_DIR=/opt/include
```

### Runtime

If USD was built as shared libraries, make sure its `lib/` directory is on
`LD_LIBRARY_PATH` (Linux), `DYLD_LIBRARY_PATH` (macOS), or `PATH` (Windows) at
runtime. The install also writes a `plugin/usd/ufg_plugin/` tree that you can
point USD's plugin loader at via `PXR_PLUGINPATH_NAME`.

## Using the Command-Line Tool

The command-line tool is called `usd_from_gltf`. Run it with (use `--help` for full documentation):

```sh
usd_from_gltf <source.gltf> <destination.usdz>
```

## Using the Library

The converter can be linked with other applications using the libraries in `<install-prefix>/lib/ufg`. Call `ufg::ConvertGltfToUsd` to convert a glTF file to USD.

## Using the Import Plugin

The plugin isn't necessary for conversion, but it's useful for previewing glTF files in `usdview`.

To use it, set the `PXR_PLUGINPATH_NAME` environment variable to the `plugin/usd/ufg_plugin/resources` directory inside the install prefix (or the parent directory if you have many plugins side-by-side).

## Compatibility

While USD is a general-purpose format, this library focuses on compatibility with [AR Quick Look](https://developer.apple.com/arkit/gallery). The [AR Quick Look](https://developer.apple.com/arkit/gallery) renderer only supports a subset of the [glTF 2.0 specification](https://github.com/KhronosGroup/glTF), so there are several limitations. Where reasonable, missing features are emulated in an effort to reproduce glTF files as faithfully as possible on iOS. The emulation can be a lossy process and the output is not well suited as an interchange format.

### Key Features
*   Reads both text (glTF) and binary (GLB) input files.
*   Generates USDA and/or USDZ output files.
*   Reads embedded binary data and [Draco](https://github.com/google/draco)-compressed meshes.
*   Rigid and skinned animation.
*   Untextured, textured, unlit and PBR lit materials.
*   glTF extensions: [KHR_materials_unlit](https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_unlit), [KHR_materials_pbrSpecularGlossiness](https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_materials_pbrSpecularGlossiness), [KHR_texture_transform](https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_texture_transform) (partially), [KHR_draco_mesh_compression](https://github.com/KhronosGroup/glTF/tree/master/extensions/2.0/Khronos/KHR_draco_mesh_compression).

### Emulated Functionality for [AR Quick Look](https://developer.apple.com/arkit/gallery)
Several rendering features of glTF are not currently supported in [AR Quick Look](https://developer.apple.com/arkit/gallery), but they are emulated where reasonable. The emulated features are:

*   Texture channel references. USD supports this, but [AR Quick Look](https://developer.apple.com/arkit/gallery) requires distinct textures for the roughness, metallic, and occlusion channels. The converter splits channels into separate textures and recompresses them as necessary.
*   Texture color scale and offset (baked into the texture).
*   Texture UV transforms (baked into vertex UVs).
*   Specular workflow approximated as metallic+roughness.
*   Arbitrary asset size limit (~200 MB on iOS) — globally resized to fit.
*   Unlit materials emulated with pure emissive material.
*   sRGB emissive texture conversion to linear.
*   Alpha cutoff baked to 0/1.
*   Double-sided geometry via duplicated geometry.
*   Normal-map renormalisation.
*   Inverted transforms baked into reversed winding.
*   Quaternion rigid animation converted to Euler (iOS 12 limitation).
*   Slerp interpolation approximated with denser linear keys.
*   Multiple skeletons merged into one.
*   Step / cubic animation modes baked to linear.
*   Vertex quantisation and Draco compression decoded to full float.

### Features Unsupported by [AR Quick Look](https://developer.apple.com/arkit/gallery)
*   Vertex colors.
*   Morph targets and vertex animation.
*   Texture filter modes (all sampled linearly with mipmapping).
*   Clamp/mirror wrap (all repeat).
*   Cameras.
*   Shadow animation (first-frame only).
*   Transparent shadows.
*   Multiple UV sets.
*   Multiple animations.
*   Multiple scenes.
*   Transparency sorting.
*   Skinned normals (baked to the first frame, so highlights look painted-on).

## Performance

`usd_from_gltf` is roughly 10-15x faster than scripted alternatives.

The bulk of the conversion time is spent in image processing and recompression, necessary for emulating otherwise unsupported functionality in [AR Quick Look](https://developer.apple.com/arkit/gallery).

### Primary Optimizations
*   Native C++17.
*   Can generate both USDA and USDZ files in a single pass.

## Troubleshooting

*   `pxrConfig.cmake` not found: re-run cmake with `-DUSD_DIR=<install prefix>` or set `CMAKE_PREFIX_PATH`.
*   Runtime "No plugins found": make sure USD's `lib/usd/plugInfo.json` is on `PXR_PLUGINPATH_NAME`, or pass `--plugin_path` to `usd_from_gltf`.
*   Linker error on `libturbojpeg`: install `libjpeg-turbo` and point `TURBOJPEG_LIBRARY` / `TURBOJPEG_INCLUDE_DIR` at it.

## License

Apache 2.0 — see [LICENSE](LICENSE).
