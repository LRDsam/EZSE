#pragma once

#include "EZSE/Renderer/Mesh.h"

#include <cstdint>
#include <vector>

namespace ezse {

// A mesh in memory, before it goes to the GPU: what a file parser makes and
// Renderer::CreateMesh takes.
struct MeshData {
    std::vector<Vertex> vertices;
    std::vector<std::uint32_t> indices;
};

} // namespace ezse
