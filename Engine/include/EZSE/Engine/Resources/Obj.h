#pragma once

#include "EZSE/Engine/Resources/MeshData.h"

#include <optional>
#include <string_view>

namespace ezse {

// Reads a Wavefront OBJ from text: v, vt and vn lines, and f lines whose corners all have the form
// position/uv/normal ("f 1/1/1 2/2/1 3/3/1"); a face with more than three corners becomes a fan of
// triangles. Other lines (comments, o, s, usemtl) are skipped. Not supported: corners without a
// uv or normal ("f 1//1"), negative indices, materials. Corners that are the same become one
// Vertex. Returns nothing (and logs the line) if the text does not fit this.
std::optional<MeshData> ParseObj(std::string_view text);

} // namespace ezse
