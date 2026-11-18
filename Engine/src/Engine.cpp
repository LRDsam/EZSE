#include "EZSE/Engine/Engine.h"

#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/MeshRenderer.h"
#include "EZSE/Engine/Components/PreviousTransform.h"
#include "EZSE/Engine/Components/Transform.h"
#include "EZSE/Engine/Components/WorldMatrix.h"
#include "EZSE/Engine/Systems/CameraSystem.h"
#include "EZSE/Engine/Systems/LightSystem.h"
#include "EZSE/Engine/Systems/RenderSystem.h"
#include "EZSE/Engine/Systems/TransformHistorySystem.h"
#include "EZSE/Engine/Systems/WorldMatrixSystem.h"

#include "EZSE/Platform/FileSystem.h"
#include "EZSE/Platform/Time.h"
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

// What the headless loop measures; logged and cleared once per second.
struct TickStats {
    int ticks = 0;
    // The longest Step: the room the simulation has left within one tick.
    std::chrono::steady_clock::duration slowestTick{0};
    // How much later than asked SleepUntil returned: how precise sleeping is.
    std::chrono::steady_clock::duration latestWake{0};
    // Ticks that took longer than a tick may take.
    int overBudget = 0;
};

void LogTickStats(const TickStats& stats) {
    using Milliseconds = std::chrono::duration<float, std::milli>;
    LogInfo("Ticks {} | slowest tick {:.2f} ms | latest wake {:.2f} ms | over budget {}",
            stats.ticks,
            Milliseconds(stats.slowestTick).count(),
            Milliseconds(stats.latestWake).count(),
            stats.overBudget);
}

} // namespace

std::unique_ptr<Engine> Engine::Create(const EngineDesc& desc) {
    // Checked first, before a window opens; also in Release, where an assert is gone and the
    // tick duration below would divide by zero.
    if (desc.ticksPerSecond <= 0) {
        LogError("Ticks per second must be higher than 0, got {}", desc.ticksPerSecond);
        return nullptr;
    }

    // Headless, both stay empty.
    std::unique_ptr<Window> window;
    std::unique_ptr<Renderer> renderer;
    if (desc.window.has_value()) {
        window = Window::Create(*desc.window);
        if (!window) {
            LogError("Failed to create window");
            return nullptr;
        }

        renderer = Renderer::Create(window->GetGLLoadFunction());
        if (!renderer) {
            LogError("Failed to create renderer");
            return nullptr;
        }
    } else {
        LogInfo("Running headless");
    }

    return std::unique_ptr<Engine>(new Engine(std::move(window), std::move(renderer), desc));
}

Engine::Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer,
               const EngineDesc& desc)
    : m_window(std::move(window)),
      m_renderer(std::move(renderer)),
      // Create has checked that ticksPerSecond is higher than 0.
      m_tickDuration(std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                         std::chrono::seconds{1}) / desc.ticksPerSecond),
      // m_tickDuration is declared before m_simulationTime, so it is already set here.
      m_simulationTime{.tickSeconds = std::chrono::duration<float>(m_tickDuration).count()} {
    if (!IsHeadless()) {
        m_resources.emplace(*m_renderer, GetExecutableDirectory() / "assets");
    }
    m_world.RegisterComponent<Transform>();
    m_world.RegisterComponent<MeshRenderer>();
    m_world.RegisterComponent<Camera>();
    m_world.RegisterComponent<Light>();
    m_world.RegisterComponent<PreviousTransform>();
    m_world.RegisterComponent<WorldMatrix>();
    // The engine adds its own systems; the group sets their place, not the order of adding.
    // PreUpdate keeps the previous Transform before the game changes it in this tick.
    m_world.AddSystem<TransformHistorySystem>(SimulationGroup::PreUpdate,
                                              MakeSignature<Transform, PreviousTransform>());
    if (!IsHeadless()) {
        InitPresentation(desc);
    }
}

Engine::~Engine() {
    // The body runs before the members are destroyed, so the Renderer and the window still exist.
    // Headless, there is no debug UI to shut down.
    if (!IsHeadless()) {
        m_renderer->ShutdownDebugUI();
        m_window->ShutdownDebugUI();
        ImGui::DestroyContext();
    }
}

void Engine::Run() {
    if (IsHeadless()) {
        RunHeadless();
    } else {
        RunWithWindow();
    }
}

void Engine::Quit() {
    m_quitRequested = true;
}

void Engine::Step() {
    m_world.Update(SystemPhase::Simulation, m_simulationTime.tickSeconds);
    ++m_simulationTime.tick;
}

bool Engine::IsHeadless() const {
    // Create makes the window and the Renderer together, or neither.
    return m_window == nullptr;
}

const SimulationTime& Engine::GetSimulationTime() const {
    return m_simulationTime;
}

World& Engine::GetWorld() {
    return m_world;
}

Window& Engine::GetWindow() {
    EZSE_ASSERT(!IsHeadless(), "There is no window when running headless");
    return *m_window;
}

Renderer& Engine::GetRenderer() {
    EZSE_ASSERT(!IsHeadless(), "There is no Renderer when running headless");
    return *m_renderer;
}

ResourceManager& Engine::GetResources() {
    EZSE_ASSERT(!IsHeadless(), "There are no resources when running headless");
    return *m_resources;
}

DebugUI& Engine::GetDebugUI() {
    EZSE_ASSERT(!IsHeadless(), "There is no debug UI when running headless");
    return m_debugUI;
}

void Engine::InitPresentation(const EngineDesc& desc) {
    // PostUpdate computes the WorldMatrix after the game changed this frame, before drawing.
    m_world.AddSystem<WorldMatrixSystem>(PresentationGroup::PostUpdate,
                                         MakeSignature<Transform>(),
                                         m_simulationTime);
    // Render, in the order of adding: the camera and the lights first, because the RenderSystem
    // draws with what they hand to the Renderer in the same frame (the view, the lights and the
    // shadow matrix).
    m_world.AddSystem<CameraSystem>(PresentationGroup::Render,
                                    MakeSignature<WorldMatrix, Camera>(),
                                    *m_window,
                                    *m_renderer);
    m_world.AddSystem<LightSystem>(PresentationGroup::Render,
                                   MakeSignature<WorldMatrix, Light>(),
                                   *m_renderer,
                                   desc.shadowExtent);
    m_world.AddSystem<RenderSystem>(PresentationGroup::Render,
                                    MakeSignature<WorldMatrix, MeshRenderer>(),
                                    *m_renderer,
                                    desc.clearColor);

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
            measured = static_cast<float>(m_simulationTime.tick - ticksAtStart) / elapsed.count();
            windowStart = now;
            ticksAtStart = m_simulationTime.tick;
        }
        const auto targetTicksPerSecond = std::chrono::seconds{1} / m_tickDuration;
        ImGui::Text(
            "Tick %llu (%.1f ticks/s, target %lld)",
            static_cast<unsigned long long>(m_simulationTime.tick),
            measured,
            static_cast<long long>(targetTicksPerSecond));
    });
    m_debugUI.AddPanel("Shadow map", [&renderer = *m_renderer](World&) {
        renderer.DrawShadowMapDebugUI();
    });
}

void Engine::RunWithWindow() {
    auto framebufferSize = m_window->GetFramebufferSize();
    m_renderer->SetViewport(framebufferSize.width, framebufferSize.height);
    auto lastFrame = std::chrono::steady_clock::now();
    // Time not yet simulated; whole ticks are taken out, the rest stays for the next frame.
    std::chrono::steady_clock::duration accumulator{0};

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
            accumulator -= m_tickDuration;
            Step();
        }
        // What is left in the accumulator is always less than one tick, so alpha stays below 1.
        m_simulationTime.alpha = std::chrono::duration<float>(accumulator) /
                                 std::chrono::duration<float>(m_tickDuration);

        m_world.Update(SystemPhase::Presentation, std::chrono::duration<float>(frameTime).count());

        m_debugUI.Draw(m_world);
        ImGui::Render();
        m_renderer->DrawDebugUI();
        m_window->SwapBuffers();
    }
}

void Engine::RunHeadless() {
    using Clock = std::chrono::steady_clock;
    // The moment the next tick is due. It moves by exactly one tick each time, so rounding in
    // sleeping does not add up over time.
    Clock::time_point nextTick = Clock::now();
    TickStats stats;
    Clock::time_point statsStart = nextTick;

    while (!m_quitRequested) {
        const Clock::time_point tickStart = Clock::now();
        Step();
        const Clock::duration tickTime = Clock::now() - tickStart;
        ++stats.ticks;
        stats.slowestTick = std::max(stats.slowestTick, tickTime);
        if (tickTime > m_tickDuration) {
            ++stats.overBudget;
        }
        // A system may have called Quit during the tick: stop without waiting for the next one.
        if (m_quitRequested) {
            break;
        }

        nextTick += m_tickDuration;
        const Clock::time_point now = Clock::now();
        if (now - nextTick > c_maxFrameTime) {
            // Too far behind (a breakpoint, a stalled disk): give up catching up, as the loop
            // with a window does. The simulation then runs behind real time.
            LogWarn("Simulation fell {} ms behind, skipping ahead",
                    std::chrono::duration_cast<std::chrono::milliseconds>(now - nextTick).count());
            nextTick = now;
        }
        // Only measured when there is something to wait for: a tick that is already due runs at
        // once (catching up), and its delay says nothing about how precise sleeping is.
        if (now < nextTick) {
            SleepUntil(nextTick);
            stats.latestWake = std::max(stats.latestWake, Clock::now() - nextTick);
        }

        if (Clock::now() - statsStart >= std::chrono::seconds{1}) {
            LogTickStats(stats);
            stats = TickStats{};
            statsStart = Clock::now();
        }
    }
}

} // namespace ezse
