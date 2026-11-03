#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/Engine/Resources/Image.h"
#include "EZSE/Engine/Resources/Obj.h"

#include "EZSE/Core/File.h"
#include "EZSE/Core/Log.h"

#include <optional>
#include <utility>

namespace ezse {

namespace {

std::string MakeKey(const std::filesystem::path& path) {
    return path.lexically_normal().generic_string();
}

} // namespace

ResourceManager::ResourceManager(Renderer& renderer, std::filesystem::path root) :
    m_renderer(renderer), m_root(std::move(root)) {}

ResourceManager::~ResourceManager() {
    for (MeshHandle mesh : m_meshes.GetHandles()) {
        m_renderer.DestroyMesh(mesh);
    }
    for (TextureHandle texture : m_textures.GetHandles()) {
        m_renderer.DestroyTexture(texture);
    }
    for (ShaderHandle shader : m_shaders.GetHandles()) {
        m_renderer.DestroyShader(shader);
    }
}

MeshHandle ResourceManager::LoadMesh(const std::filesystem::path& path) {
    const std::string key = MakeKey(path);
    MeshHandle mesh = m_meshes.Acquire(key);
    if (mesh.id != 0) {
        return mesh;
    }
    std::optional<MeshData> meshData = ReadObjFile(m_root / path);
    if (!meshData.has_value()) {
        return MeshHandle{};
    }
    mesh = m_renderer.CreateMesh(meshData->vertices, meshData->indices);
    m_meshes.Add(key, mesh);
    LogInfo("Loaded mesh '{}' (id {})", key, mesh.id);
    return mesh;
}

void ResourceManager::UnloadMesh(MeshHandle mesh) {
    const std::string key = m_meshes.GetKey(mesh);
    if (m_meshes.Release(mesh)) {
        LogInfo("Unloaded mesh '{}' (id {})", key, mesh.id);
        m_renderer.DestroyMesh(mesh);
    }
}

TextureHandle ResourceManager::LoadTexture(const std::filesystem::path& path) {
    const std::string key = MakeKey(path);
    TextureHandle texture = m_textures.Acquire(key);
    if (texture.id != 0) {
        return texture;
    }
    const std::optional<Image> image = ReadImageFile(m_root / path);
    if (!image.has_value()) {
        return TextureHandle{};
    }
    texture = m_renderer.CreateTexture(image->width, image->height, image->pixels);
    m_textures.Add(key, texture);
    LogInfo("Loaded texture '{}' (id {})", key, texture.id);
    return texture;
}

void ResourceManager::UnloadTexture(TextureHandle texture) {
    const std::string key = m_textures.GetKey(texture);
    if (m_textures.Release(texture)) {
        LogInfo("Unloaded texture '{}' (id {})", key, texture.id);
        m_renderer.DestroyTexture(texture);
    }
}

ShaderHandle ResourceManager::LoadShader(const std::filesystem::path& vertexPath,
                                         const std::filesystem::path& fragmentPath) {
    const std::string key = MakeKey(vertexPath) + '|' + MakeKey(fragmentPath);
    ShaderHandle shader = m_shaders.Acquire(key);
    if (shader.id != 0) {
        return shader;
    }
    const std::optional<std::string> vertexSource = ReadTextFile(m_root / vertexPath);
    const std::optional<std::string> fragmentSource = ReadTextFile(m_root / fragmentPath);
    if (!vertexSource.has_value() || !fragmentSource.has_value()) {
        return ShaderHandle{};
    }
    shader = m_renderer.CreateShader(*vertexSource, *fragmentSource);
    if (shader.id == 0) {
        return ShaderHandle{};
    }
    m_shaders.Add(key, shader);
    LogInfo("Loaded shader '{}' (id {})", key, shader.id);
    return shader;
}

void ResourceManager::UnloadShader(ShaderHandle shader) {
    const std::string key = m_shaders.GetKey(shader);
    if (m_shaders.Release(shader)) {
        LogInfo("Unloaded shader '{}' (id {})", key, shader.id);
        m_renderer.DestroyShader(shader);
    }
}

} // namespace ezse
