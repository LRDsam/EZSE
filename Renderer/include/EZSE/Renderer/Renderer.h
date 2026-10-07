#pragma once

#include "EZSE/Renderer/Mesh.h"

#include <memory>
#include <span>
#include <vector>


namespace ezse {

using GLProc = void (*)();
using GLLoadFunction = GLProc (*)(const char* name);

class Renderer {
public:
    static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);

    
    MeshHandle CreateMesh(std::span<const Vertex> vertices);
    void DrawMesh(MeshHandle mesh);
    void DestroyMesh(MeshHandle mesh);

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

    MeshData& GetMeshData(MeshHandle mesh);

    std::vector<MeshData> m_meshData;

    // Will be deleted soon
    unsigned int m_shader = 0;
};

} // namespace ezse
