/*
 * Copyright 2019 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef USDGLTF_FILE_FORMAT_H
#define USDGLTF_FILE_FORMAT_H

#ifdef _MSC_VER
// Disable warnings in the pxr headers.
// conversion from 'double' to 'float', possible loss of data
#pragma warning(disable : 4244)
// conversion from 'size_t' to 'int', possible loss of data
#pragma warning(disable : 4267)
// truncation from 'double' to 'float'
#pragma warning(disable : 4305)
#endif  // _MSC_VER

#include <iosfwd>
#include <string>
#include "pxr/usd/sdf/fileFormat.h"
#include "pxr/base/tf/staticTokens.h"

PXR_NAMESPACE_OPEN_SCOPE

#define USDGLTF_FILE_FORMAT_TOKENS \
  ((Id,      "gltf"))              \
  ((Version, "1.0"))               \
  ((Target,  "usd"))

TF_DECLARE_PUBLIC_TOKENS(UsdGltfFileFormatTokens, USDGLTF_FILE_FORMAT_TOKENS);

TF_DECLARE_WEAK_AND_REF_PTRS(UsdGltfFileFormat);

class UsdGltfFileFormat : public SdfFileFormat {
 public:
  bool CanRead(const std::string &file) const override;
  bool Read(SdfLayer* layer,
            const std::string& resolved_path,
            bool metadata_only) const override;
  bool ReadFromString(SdfLayer* layer, const std::string& str) const override;
  bool WriteToString(const SdfLayer& layer,
                     std::string* str,
                     const std::string& comment = std::string()) const override;
  bool WriteToStream(const SdfSpecHandle& spec, std::ostream& out,
                     size_t indent) const override;

 protected:
  SDF_FILE_FORMAT_FACTORY_ACCESS;
  ~UsdGltfFileFormat() override;
  UsdGltfFileFormat();
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif  // USDGLTF_FILE_FORMAT_H
