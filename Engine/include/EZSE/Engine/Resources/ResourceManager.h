#pragma once

#include "EZSE/Engine/Resources/ResourceCache.h"

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Shader.h"
#include "EZSE/Renderer/Texture.h"

#include <filesystem>
#include <string>

namespace ezse {

class ResourceManager {
public:
    ResourceManager(Renderer& renderer, std::filesystem::path root);
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    MeshHandle LoadMesh(const std::filesystem::path& path);
    void UnloadMesh(MeshHandle mesh);

    TextureHandle LoadTexture(const std::filesystem::path& path);
    void UnloadTexture(TextureHandle texture);

    ShaderHandle LoadShader(const std::filesystem::path& vertexPath,
                            const std::filesystem::path& fragmentPath);
    void UnloadShader(ShaderHandle shader);

private:
    Renderer& m_renderer;
    std::filesystem::path m_root;

    ResourceCache<MeshHandle> m_meshes;
    ResourceCache<TextureHandle> m_textures;
    ResourceCache<ShaderHandle> m_shaders;
};

} // namespace ezse
