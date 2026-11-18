#pragma once

#include "EZSE/Engine/Debug/DebugUI.h"
#include "EZSE/Engine/Resources/ResourceManager.h"
#include "EZSE/Engine/SimulationTime.h"

#include "EZSE/ECS/World.h"
#include "EZSE/Math/Vec3.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>

namespace ezse {

struct EngineDesc {
    // The window to open. Empty runs headless, as a server does: no window, Renderer, resources
    // or debug UI, and only SystemPhase::Simulation runs.
    std::optional<WindowDesc> window = WindowDesc{};
    // How often the simulation phase runs. Client and server must use the same value.
    int ticksPerSecond = 30;
    // The background the screen is cleared to each frame: red, green and blue in x, y and z,
    // from 0 to 1.
    Vec3 clearColor = {0.010f, 0.033f, 0.073f};
    // Half the size, in meters, of the cube around the origin in which the sun casts shadows;
    // 10 gives a cube of 20 meters.
    float shadowExtent = 10.0f;
};

class Engine {
public:
    // Registers the built-in components (Transform, MeshRenderer, Camera, Light, PreviousTransform,
    // WorldMatrix) in its World and adds its own systems, each in its group: the transform
    // history, the WorldMatrix, and the camera, lights and drawing in PresentationGroup::Render.
    // The game registers and adds only its own components and systems. Headless, the Engine adds
    // only the transform history: the WorldMatrix and drawing belong to the presentation.
    static std::unique_ptr<Engine> Create(const EngineDesc& desc);

    ~Engine();

    void Run();
    void Quit();
    // Runs exactly one tick of SystemPhase::Simulation and counts it, without looking at real time.
    // Run calls it for every tick; tests (and later bots and a server inside the client) call it
    // directly. Not from inside a system: the tick would run within itself.
    void Step();

    // Whether the Engine runs without a window (EngineDesc::window was empty).
    bool IsHeadless() const;

    const SimulationTime& GetSimulationTime() const;

    World& GetWorld();
    // Only with a window: these assert when running headless.
    Window& GetWindow();
    Renderer& GetRenderer();
    ResourceManager& GetResources();
    DebugUI& GetDebugUI();

private:
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer,
           const EngineDesc& desc);

    // The part that needs a window: the WorldMatrix, camera, lights, drawing and the debug UI.
    void InitPresentation(const EngineDesc& desc);

    // Destroyed in reverse order:
    // the world (and its systems) first, then the resources, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    // Empty when running headless: it uploads to the GPU through the Renderer.
    std::optional<ResourceManager> m_resources;
    DebugUI m_debugUI;
    World m_world;
    bool m_quitRequested = false;
    std::chrono::steady_clock::duration m_tickDuration;
    SimulationTime m_simulationTime;
};

} // namespace ezse
