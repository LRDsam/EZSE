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

// Loads meshes, textures and shaders by virtual path, once per path, with a counter per handle.
// A load that fails returns a fallback and logs a warning, so the game keeps running and the
// missing asset is easy to spot: a magenta and black checkerboard for a texture, a plain magenta
// shader, a cube for a mesh. A load never returns id 0. Unloading a fallback does nothing.
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
    // Log a warning for the path and return the fallback.
    MeshHandle UseFallbackMesh(std::string_view path) const;
    TextureHandle UseFallbackTexture(std::string_view path) const;
    ShaderHandle UseFallbackShader(std::string_view vertexPath,
                                   std::string_view fragmentPath) const;

    Renderer& m_renderer;
    const VirtualFileSystem& m_fileSystem;

    ResourceCache<MeshHandle> m_meshes;
    ResourceCache<TextureHandle> m_textures;
    ResourceCache<ShaderHandle> m_shaders;

    // Made in code by the constructor, not read from files; they live as long as the manager.
    MeshHandle m_fallbackMesh;
    TextureHandle m_fallbackTexture;
    ShaderHandle m_fallbackShader;
};

} // namespace ezse
