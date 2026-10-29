#pragma once

#include "EZSE/Renderer/Shader.h"
#include "EZSE/Renderer/Texture.h"

#include "EZSE/Math/Vec3.h"

namespace ezse {

struct Material {
    ShaderHandle shader;
    TextureHandle texture; // id 0 equals a white texture.
    Vec3 tint = {1.0f, 1.0f, 1.0f}; // multiplies the texture color
    float shininess = 128.0f;
    float specularStrength = 0.5f;
};

} // namespace ezse
