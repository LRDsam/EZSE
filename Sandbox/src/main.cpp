#include "EZSE/Core/Log.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"
#include "EZSE/Renderer/Mesh.h"

int main() {
    auto window = ezse::Window::Create({.title = "EZSE Sandbox"});
    if (!window) {
        return 1;
    }

    auto renderer = ezse::Renderer::Create(window->GetGLLoadFunction());
    if (!renderer) {
        return 1;
    }

    const ezse::Vertex triangle[] = {
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{ 0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
    };
    const ezse::Vertex triangle2[] = {
       {{-0.5f,  0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
       {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
       {{ 0.5f,  0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}},
    };

    ezse::MeshHandle mesh = renderer->CreateMesh(triangle);
    ezse::MeshHandle mesh2 = renderer->CreateMesh(triangle2);

   
    while (!window->ShouldClose()) {
        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        renderer->DrawMesh(mesh);
        renderer->DrawMesh(mesh2);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
