#pragma once

#include <memory>

namespace ezse {

using GLProc = void (*)();
using GLLoadFunction = GLProc (*)(const char* name);

class Renderer {
public:
    static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);
    void DrawTriangle();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;
    ~Renderer();

    void Clear(float red, float green, float blue);

private:
    Renderer() = default;
    unsigned int m_vertexBuffer = 0;
    unsigned int m_vao = 0;
    unsigned int m_shader = 0;
};

} // namespace ezse
