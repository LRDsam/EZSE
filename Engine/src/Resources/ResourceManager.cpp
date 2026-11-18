#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/Engine/Resources/Image.h"
#include "EZSE/Engine/Resources/Obj.h"

#include "EZSE/Core/Log.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ezse {

ResourceManager::ResourceManager(Renderer& renderer, const VirtualFileSystem& fileSystem) :
    m_renderer(renderer), m_fileSystem(fileSystem) {}

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

MeshHandle ResourceManager::LoadMesh(std::string_view path) {
    // The virtual path is the key: the path rules already make it unambiguous.
    const std::string key(path);
    MeshHandle mesh = m_meshes.Acquire(key);
    if (mesh.id != 0) {
        return mesh;
    }
    std::optional<std::string> text = m_fileSystem.ReadText(path);
    if (!text.has_value()) {
        return MeshHandle{};
    }
    // ReadText logs why reading failed; ParseObj logs the line, but not which file it was.
    std::optional<MeshData> meshData = ParseObj(text.value());
    if (!meshData.has_value()) {
        LogError("Failed to load mesh '{}'", key);
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

TextureHandle ResourceManager::LoadTexture(std::string_view path) {
    const std::string key(path);
    TextureHandle texture = m_textures.Acquire(key);
    if (texture.id != 0) {
        return texture;
    }
    std::optional<std::vector<std::uint8_t>> bytes = m_fileSystem.ReadBinary(path);
    if (!bytes.has_value()) {
        return TextureHandle{};
    }
    // DecodeImage logs the reason, but not which file it was.
    std::optional<Image> image = DecodeImage(bytes.value());
    if (!image.has_value()) {
        LogError("Failed to load texture '{}'", key);
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

ShaderHandle ResourceManager::LoadShader(std::string_view vertexPath,
                                         std::string_view fragmentPath) {
    // '|' is not allowed in a virtual path, so two different pairs never give the same key.
    const std::string key = std::string(vertexPath) + '|' + std::string(fragmentPath);
    ShaderHandle shader = m_shaders.Acquire(key);
    if (shader.id != 0) {
        return shader;
    }
    const std::optional<std::string> vertexSource = m_fileSystem.ReadText(vertexPath);
    const std::optional<std::string> fragmentSource = m_fileSystem.ReadText(fragmentPath);
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
