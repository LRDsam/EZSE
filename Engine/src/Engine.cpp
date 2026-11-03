#include "EZSE/Engine/Engine.h"

#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Platform/FileSystem.h"
#include "EZSE/Platform/WindowEvent.h"

#include "EZSE/Core/Log.h"

#include <imgui.h>

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
    : m_window(std::move(window)),
      m_renderer(std::move(renderer)),
      m_resources(*m_renderer, GetExecutableDirectory() / "assets") {
    m_world.RegisterComponent<Transform>();
    m_world.RegisterComponent<MeshRenderer>();
    m_world.RegisterComponent<Camera>();

    ImGui::CreateContext();
    // No imgui.ini with window positions in the working folder.
    ImGui::GetIO().IniFilename = nullptr;
    m_window->InitDebugUI();
    m_renderer->InitDebugUI();
    m_debugUI.AddPanel("Stats", [](World&) {
        const float framerate = ImGui::GetIO().Framerate;
        ImGui::Text("%.2f ms (%.0f FPS)", 1000.0f / framerate, framerate);
    });
}

Engine::~Engine() {
    // The body runs before the members are destroyed, so the Renderer and the window still exist.
    m_renderer->ShutdownDebugUI();
    m_window->ShutdownDebugUI();
    ImGui::DestroyContext();
}

void Engine::Run() {
    auto framebufferSize = m_window->GetFramebufferSize();
    m_renderer->SetViewport(framebufferSize.width, framebufferSize.height);
    auto lastFrame = std::chrono::steady_clock::now();

    while (!m_quitRequested && !m_window->ShouldClose()) {
        m_window->PollEvents();
        m_window->BeginDebugUIFrame();
        m_renderer->BeginDebugUIFrame();
        ImGui::NewFrame();
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

        // Systems may call ImGui:: functions during the update.
        m_world.Update(deltaTime.count());

        m_debugUI.Draw(m_world);
        ImGui::Render();
        m_renderer->DrawDebugUI();
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

ResourceManager& Engine::GetResources() {
    return m_resources;
}

DebugUI& Engine::GetDebugUI() {
    return m_debugUI;
}

} // namespace ezse
