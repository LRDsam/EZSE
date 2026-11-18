#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/Engine/Resources/Image.h"
#include "EZSE/Engine/Resources/Obj.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace ezse {

namespace {

// The cube of engine/models/cube.obj, in code so the fallback does not depend on a file.
constexpr std::string_view c_fallbackCubeObj = R"(
v -0.5 -0.5 0.5
v 0.5 -0.5 0.5
v 0.5 0.5 0.5
v -0.5 0.5 0.5
v 0.5 -0.5 -0.5
v -0.5 -0.5 -0.5
v -0.5 0.5 -0.5
v 0.5 0.5 -0.5
vt 0 0
vt 1 0
vt 1 1
vt 0 1
vn 0 0 1
vn 0 0 -1
vn 1 0 0
vn -1 0 0
vn 0 1 0
vn 0 -1 0
f 1/1/1 2/2/1 3/3/1 4/4/1
f 5/1/2 6/2/2 7/3/2 8/4/2
f 2/1/3 5/2/3 8/3/3 3/4/3
f 6/1/4 1/2/4 4/3/4 7/4/4
f 4/1/5 3/2/5 8/3/5 7/4/5
f 6/1/6 5/2/6 2/3/6 1/4/6
)";

// The vertex stage of engine/shaders/basic.vert without the UV. The Renderer also sets uniforms
// this shader does not have (tint, shininess); OpenGL skips those.
constexpr std::string_view c_fallbackVertexSource = R"(#version 460 core
layout(location = 0) in vec3 a_position;
layout(std140, binding = 0) uniform Camera {
    mat4 u_viewProjection;
};
uniform mat4 u_model;
void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 1.0);
}
)";

// Plain magenta, without light or tint, so it stands out in any scene.
constexpr std::string_view c_fallbackFragmentSource = R"(#version 460 core
out vec4 fragColor;
void main() {
    fragColor = vec4(1.0, 0.0, 1.0, 1.0);
}
)";

// 64 by 64 pixels in squares of 8. Not 2 by 2: textures are filtered linearly, which would blur
// so few pixels into a gradient.
constexpr int c_fallbackTextureSize = 64;
constexpr int c_fallbackSquareSize = 8;

Image MakeFallbackImage() {
    Image image{.width = c_fallbackTextureSize, .height = c_fallbackTextureSize};
    image.pixels.reserve(static_cast<std::size_t>(c_fallbackTextureSize) *
                         c_fallbackTextureSize * 4);
    for (int y = 0; y < c_fallbackTextureSize; ++y) {
        for (int x = 0; x < c_fallbackTextureSize; ++x) {
            const bool magenta = (x / c_fallbackSquareSize + y / c_fallbackSquareSize) % 2 == 0;
            const std::uint8_t redAndBlue = magenta ? 255 : 0;
            image.pixels.insert(image.pixels.end(), {redAndBlue, 0, redAndBlue, 255});
        }
    }
    return image;
}

} // namespace

ResourceManager::ResourceManager(Renderer& renderer, const VirtualFileSystem& fileSystem) :
    m_renderer(renderer), m_fileSystem(fileSystem) {
    // Our own data without input from outside: a failure here is a bug in the engine.
    const std::optional<MeshData> cube = ParseObj(c_fallbackCubeObj);
    EZSE_ASSERT(cube.has_value(), "The fallback cube does not parse");
    m_fallbackMesh = m_renderer.CreateMesh(cube->vertices, cube->indices);

    const Image image = MakeFallbackImage();
    m_fallbackTexture = m_renderer.CreateTexture(image.width, image.height, image.pixels);

    m_fallbackShader = m_renderer.CreateShader(c_fallbackVertexSource, c_fallbackFragmentSource);
    EZSE_ASSERT(m_fallbackShader.id != 0, "The fallback shader does not compile");
}

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
    m_renderer.DestroyMesh(m_fallbackMesh);
    m_renderer.DestroyTexture(m_fallbackTexture);
    m_renderer.DestroyShader(m_fallbackShader);
}

MeshHandle ResourceManager::LoadMesh(std::string_view path) {
    // The virtual path is the key: the path rules already make it unambiguous.
    const std::string key(path);
    MeshHandle mesh = m_meshes.Acquire(key);
    if (mesh.id != 0) {
        return mesh;
    }
    // A failed load is not cached: the next Load of the same path tries again.
    std::optional<std::string> text = m_fileSystem.ReadText(path);
    if (!text.has_value()) {
        return UseFallbackMesh(path);
    }
    // ReadText logs why reading failed; ParseObj logs the line, but not which file it was.
    std::optional<MeshData> meshData = ParseObj(text.value());
    if (!meshData.has_value()) {
        LogError("Failed to load mesh '{}'", key);
        return UseFallbackMesh(path);
    }
    mesh = m_renderer.CreateMesh(meshData->vertices, meshData->indices);
    m_meshes.Add(key, mesh);
    LogInfo("Loaded mesh '{}' (id {})", key, mesh.id);
    return mesh;
}

void ResourceManager::UnloadMesh(MeshHandle mesh) {
    if (mesh.id == m_fallbackMesh.id) {
        return;
    }
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
        return UseFallbackTexture(path);
    }
    // DecodeImage logs the reason, but not which file it was.
    std::optional<Image> image = DecodeImage(bytes.value());
    if (!image.has_value()) {
        LogError("Failed to load texture '{}'", key);
        return UseFallbackTexture(path);
    }
    texture = m_renderer.CreateTexture(image->width, image->height, image->pixels);
    m_textures.Add(key, texture);
    LogInfo("Loaded texture '{}' (id {})", key, texture.id);
    return texture;
}

void ResourceManager::UnloadTexture(TextureHandle texture) {
    if (texture.id == m_fallbackTexture.id) {
        return;
    }
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
        return UseFallbackShader(vertexPath, fragmentPath);
    }
    // CreateShader logs the compile or link error.
    shader = m_renderer.CreateShader(*vertexSource, *fragmentSource);
    if (shader.id == 0) {
        return UseFallbackShader(vertexPath, fragmentPath);
    }
    m_shaders.Add(key, shader);
    LogInfo("Loaded shader '{}' (id {})", key, shader.id);
    return shader;
}

void ResourceManager::UnloadShader(ShaderHandle shader) {
    if (shader.id == m_fallbackShader.id) {
        return;
    }
    const std::string key = m_shaders.GetKey(shader);
    if (m_shaders.Release(shader)) {
        LogInfo("Unloaded shader '{}' (id {})", key, shader.id);
        m_renderer.DestroyShader(shader);
    }
}

MeshHandle ResourceManager::UseFallbackMesh(std::string_view path) const {
    LogWarn("Using the fallback mesh for '{}'", path);
    return m_fallbackMesh;
}

TextureHandle ResourceManager::UseFallbackTexture(std::string_view path) const {
    LogWarn("Using the fallback texture for '{}'", path);
    return m_fallbackTexture;
}

ShaderHandle ResourceManager::UseFallbackShader(std::string_view vertexPath,
                                                std::string_view fragmentPath) const {
    LogWarn("Using the fallback shader for '{}|{}'", vertexPath, fragmentPath);
    return m_fallbackShader;
}

} // namespace ezse
