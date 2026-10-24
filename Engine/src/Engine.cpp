#include "EZSE/Engine/Engine.h"

#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Platform/WindowEvent.h"

#include "EZSE/Core/Log.h"

#include <chrono>
#include <variant>

namespace ezse {

std::unique_ptr<Engine> Engine::Create(const WindowDesc& windowDesc) {
    auto window = Window::Create(windowDesc);
    if (!window) {
        LogError("Failed to create window");
        return nullptr;
    }

    auto renderer = Renderer::Create(window->GetGLLoadFunction());
    if (!renderer) {
        LogError("Failed to create renderer");
        return nullptr;
    }

    return std::unique_ptr<Engine>(new Engine(std::move(window), std::move(renderer)));
}

Engine::Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer)
    : m_window(std::move(window)), m_renderer(std::move(renderer)) {
    m_world.RegisterComponent<Transform>();
    m_world.RegisterComponent<MeshRenderer>();
    m_world.RegisterComponent<Camera>();
}

void Engine::Run() {
    auto framebufferSize = m_window->GetFramebufferSize();
    m_renderer->SetViewport(framebufferSize.width, framebufferSize.height);
    auto lastFrame = std::chrono::steady_clock::now();

    while (!m_quitRequested && !m_window->ShouldClose()) {
        m_window->PollEvents();
        for (const WindowEvent& event : m_window->GetEvents()) {
            if (const auto* resized = std::get_if<FramebufferResized>(&event)) {
                if (resized->width == 0 || resized->height == 0) {
                    continue;
                }
                m_renderer->SetViewport(resized->width, resized->height);
            }
        }

        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> deltaTime = now - lastFrame;
        lastFrame = now;

        m_world.Update(deltaTime.count());
        m_window->SwapBuffers();
    }
}

void Engine::Quit() {
    m_quitRequested = true;
}

World& Engine::GetWorld() {
    return m_world;
}

Window& Engine::GetWindow() {
    return *m_window;
}

Renderer& Engine::GetRenderer() {
    return *m_renderer;
}

} // namespace ezse
