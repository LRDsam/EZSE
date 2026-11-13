#include "EZSE/Engine/Engine.h"

#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/Platform/FileSystem.h"
#include "EZSE/Platform/WindowEvent.h"

#include "EZSE/Core/Log.h"

#include <imgui.h>

#include <algorithm>
#include <chrono>
#include <variant>

namespace ezse {

namespace {

// Longer frames (a dragged window, a breakpoint) are cut off, so the loop never has to catch up
// on hundreds of ticks at once. The simulation then runs behind real time for a moment.
// The same type as the frame time, so std::min can compare them.
constexpr std::chrono::steady_clock::duration c_maxFrameTime = std::chrono::milliseconds{250};

} // namespace

std::unique_ptr<Engine> Engine::Create(const EngineDesc& desc) {
    // Checked first, before a window opens; also in Release, where an assert is gone and the
    // tick duration below would divide by zero.
    if (desc.ticksPerSecond <= 0) {
        LogError("Ticks per second must be higher than 0, got {}", desc.ticksPerSecond);
        return nullptr;
    }

    auto window = Window::Create(desc.window);
    if (!window) {
        LogError("Failed to create window");
        return nullptr;
    }

    auto renderer = Renderer::Create(window->GetGLLoadFunction());
    if (!renderer) {
        LogError("Failed to create renderer");
        return nullptr;
    }

    const std::chrono::steady_clock::duration tickDuration =
        std::chrono::duration_cast<std::chrono::steady_clock::duration>(std::chrono::seconds{1}) / desc.ticksPerSecond;

    return std::unique_ptr<Engine>(new Engine(std::move(window), std::move(renderer), tickDuration));
}

Engine::Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer,
               std::chrono::steady_clock::duration tickDuration)
    : m_window(std::move(window)),
      m_renderer(std::move(renderer)),
      m_resources(*m_renderer, GetExecutableDirectory() / "assets"),
      m_tickDuration(tickDuration) {
    m_world.RegisterComponent<Transform>();
    m_world.RegisterComponent<MeshRenderer>();
    m_world.RegisterComponent<Camera>();
    m_world.RegisterComponent<Light>();

    ImGui::CreateContext();
    // No imgui.ini with window positions in the working folder.
    ImGui::GetIO().IniFilename = nullptr;
    m_window->InitDebugUI();
    m_renderer->InitDebugUI();
    // The init-captures belong to the lambda and keep their values between frames; mutable lets
    // it change them.
    m_debugUI.AddPanel("Stats", [this,
                                 windowStart = std::chrono::steady_clock::now(),
                                 ticksAtStart = std::uint64_t{0},
                                 measured = 0.0f](World&) mutable {
        const float framerate = ImGui::GetIO().Framerate;
        ImGui::Text("%.2f ms (%.0f FPS)", 1000.0f / framerate, framerate);

        // Measured over windows of one second, so the number is readable and shows when the
        // simulation cannot keep up with real time.
        const auto now = std::chrono::steady_clock::now();
        const std::chrono::duration<float> elapsed = now - windowStart;
        if (elapsed.count() >= 1.0f) {
            measured = static_cast<float>(m_tickCount - ticksAtStart) / elapsed.count();
            windowStart = now;
            ticksAtStart = m_tickCount;
        }
        const auto targetTicksPerSecond = std::chrono::seconds{1} / m_tickDuration;
        ImGui::Text("Tick %llu (%.1f ticks/s, target %lld)", static_cast<unsigned long long>(m_tickCount),
                    measured, static_cast<long long>(targetTicksPerSecond));
    });
    m_debugUI.AddPanel("Shadow map", [&renderer = *m_renderer](World&) {
        renderer.DrawShadowMapDebugUI();
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
    // Time not yet simulated; whole ticks are taken out, the rest stays for the next frame.
    std::chrono::steady_clock::duration accumulator{0};
    const float tickSeconds = std::chrono::duration<float>(m_tickDuration).count();

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
        const std::chrono::steady_clock::duration frameTime = std::min(now - lastFrame, c_maxFrameTime);
        lastFrame = now;

        // Systems may call ImGui:: functions during the update.
        // The simulation runs zero or more fixed ticks, depending on the time that has passed.
        accumulator += frameTime;
        while (accumulator >= m_tickDuration) {
            m_world.Update(SystemPhase::Simulation, tickSeconds);
            accumulator -= m_tickDuration;
            ++m_tickCount;
        }

        m_world.Update(SystemPhase::Presentation, std::chrono::duration<float>(frameTime).count());

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
