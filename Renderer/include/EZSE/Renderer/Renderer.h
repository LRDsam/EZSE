#pragma once

#include <memory>

namespace ezse {

using GLProc = void (*)();
using GLLoadFunction = GLProc (*)(const char* name);

class Renderer {
public:
    static std::unique_ptr<Renderer> Create(GLLoadFunction loadFunction);

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void Clear(float red, float green, float blue);

private:
    Renderer() = default;
};

} // namespace ezse
