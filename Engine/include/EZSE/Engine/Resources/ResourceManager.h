#pragma once

#include "EZSE/Core/VirtualFileSystem.h"

#include "EZSE/Engine/Resources/ResourceCache.h"

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Shader.h"
#include "EZSE/Renderer/Texture.h"

#include <string>
#include <string_view>

namespace ezse {

class ResourceManager {
public:
    ResourceManager(Renderer& renderer, const VirtualFileSystem& fileSystem);
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    MeshHandle LoadMesh(std::string_view path);
    void UnloadMesh(MeshHandle mesh);

    TextureHandle LoadTexture(std::string_view path);
    void UnloadTexture(TextureHandle texture);

    ShaderHandle LoadShader(std::string_view vertexPath, std::string_view fragmentPath);
    void UnloadShader(ShaderHandle shader);

private:
    Renderer& m_renderer;
    const VirtualFileSystem& m_fileSystem;

    ResourceCache<MeshHandle> m_meshes;
    ResourceCache<TextureHandle> m_textures;
    ResourceCache<ShaderHandle> m_shaders;
};

} // namespace ezse
