#pragma once

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Shader.h"
#include "EZSE/Renderer/Texture.h"

namespace ezse {

struct MeshRenderer {
    MeshHandle mesh;
    ShaderHandle shader;
    TextureHandle texture; // id 0: the Renderer draws with a 1x1 white texture
};

} // namespace ezse
