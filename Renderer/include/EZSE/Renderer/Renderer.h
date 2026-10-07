#pragma once

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Shader.h"

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

    MeshHandle CreateMesh(std::span<const Vertex> vertices);
    void DrawMesh(MeshHandle mesh, ShaderHandle shader);
    void DestroyMesh(MeshHandle mesh);

    ShaderHandle CreateShader(std::string_view vertexSource, std::string_view fragmentSource);
    void DestroyShader(ShaderHandle shader);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    ~Renderer();

    void Clear(float red, float green, float blue);

private:
    Renderer() = default;

    struct MeshData {
        unsigned int vertexBufferObj = 0;
        unsigned int vertexArrayObj = 0;
        unsigned int vertexCount = 0;
    };

    struct ShaderData {
        unsigned int shaderProgram = 0;
    };

    MeshData& GetMeshData(MeshHandle mesh);
    ShaderData& GetShaderData(ShaderHandle shader);

    std::vector<MeshData> m_meshData;
    std::vector<ShaderData> m_shaderData;
};

} // namespace ezse
