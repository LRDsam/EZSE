#pragma once

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Shader.h"

namespace ezse {

struct MeshRenderer {
    MeshHandle mesh;
    ShaderHandle shader;
};

} // namespace ezse
