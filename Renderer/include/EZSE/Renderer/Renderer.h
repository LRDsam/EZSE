#pragma once

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Shader.h"
#include "EZSE/Renderer/Texture.h"
#include "EZSE/Renderer/Material.h"
#include "EZSE/Renderer/PointLight.h"


#include "EZSE/Math/Transform.h"
#include "EZSE/Math/Vec3.h"

#include <memory>
#include <span>
#include <vector>
#include <string_view>


namespace ezse {

using GLProc = void (*)();
using GLLoadFunction = GLProc (*)(const char* name);

class Renderer {
public:
    static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);
    void SetViewport(int width, int height);

    MeshHandle CreateMesh(std::span<const Vertex> vertices, std::span<const std::uint32_t> indices);
    // texture: bound to texture unit 0 for this draw; id 0 binds a 1x1 white texture.
    void DrawMesh(MeshHandle mesh, const Material& material, const Mat4& model);
    void DestroyMesh(MeshHandle mesh);

    // Creates a texture from RGBA pixels, 4 bytes per pixel, row by row (the first row is the bottom).
    // width, height: in pixels, both larger than 0. pixels: exactly width * height * 4 bytes.
    TextureHandle CreateTexture(int width, int height, std::span<const std::uint8_t> pixels);
    void DestroyTexture(TextureHandle texture);

    ShaderHandle CreateShader(std::string_view vertexSource, std::string_view fragmentSource);
    void DestroyShader(ShaderHandle shader);

    // The camera for all shaders. Stays until the next call.
    // position: where the camera stands in the world; the shaders need it for specular light.
    void SetCamera(const Mat4& view, const Mat4& projection, const Vec3& position);

    // The light that shines in one direction everywhere, like the sun. Stays until the next call.
    // direction: where the light goes (from the light to the surface); does not need length 1.
    // color: red, green and blue, usually 0 to 1.
    void SetDirectionalLight(const Vec3& direction, const Vec3& color);

    // The light that reaches every surface from all sides, so the side away from the light is
    // not black. color: red, green and blue, usually small (0.1 to 0.2). Stays until the next call.
    void SetAmbientLight(const Vec3& color);

    // All point lights for this frame; only the first 16 are drawn (a warning is logged once).
    // Stays until the next call.
    void SetPointLights(std::span<const PointLight> lights);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    ~Renderer();

    void Clear(float red, float green, float blue);

    // Debug UI (Dear ImGui), drawn over the image. InitDebugUI once after ImGui::CreateContext;
    // BeginDebugUIFrame at the start of a frame; DrawDebugUI after ImGui::Render.
    void InitDebugUI();
    void BeginDebugUIFrame();
    void DrawDebugUI();
    void ShutdownDebugUI();

private:
    Renderer() = default;

    struct GPUMesh {
        unsigned int vertexBufferObj = 0;
        unsigned int vertexArrayObj = 0;
        unsigned int indexBufferObj = 0;
        unsigned int indexCount = 0;
    };

    struct GPUShader {
        unsigned int shaderProgram = 0;
        int modelLocation = -1;
        int tintLocation = -1;
        int shininessLocation = -1;
        int specularStrengthLocation = -1;
    };

    struct GPUTexture {
        unsigned int textureObj = 0;
    };

    GPUMesh& GetGPUMesh(MeshHandle mesh);
    GPUShader& GetGPUShader(ShaderHandle shader);
    GPUTexture& GetGPUTexture(TextureHandle texture);

    unsigned int m_cameraBuffer = 0;
    unsigned int m_lightBuffer = 0;
    unsigned int m_pointLightBuffer = 0;
    // Set after the first warning about too many point lights, so it is logged only once.
    bool m_pointLightWarningLogged = false;
    // 1x1 white, drawn when a draw call has no texture
    TextureHandle m_whiteTexture;

    std::vector<GPUMesh> m_gpuMeshes;
    std::vector<GPUShader> m_gpuShaders;
    std::vector<GPUTexture> m_gpuTextures;

};

} // namespace ezse
