#include "EZSE/Core/Log.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

int main() {
    auto window = ezse::Window::Create({.title = "EZSE Sandbox"});
    if (!window) {
        return 1;
    }

    auto renderer = ezse::Renderer::Create(window->GetGLLoadFunction());
    if (!renderer) {
        return 1;
    }

    while (!window->ShouldClose()) {
        window->PollEvents();
        renderer->Clear(0.1f, 0.2f, 0.3f);
        window->SwapBuffers();
    }

    ezse::LogInfo("Sandbox closed");
    return 0;
}
