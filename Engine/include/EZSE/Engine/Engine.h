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

namespace ezse {

struct EngineDesc {
    WindowDesc window;
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
    // The game registers and adds only its own components and systems.
    static std::unique_ptr<Engine> Create(const EngineDesc& desc);

    ~Engine();

    void Run();
    void Quit();

    const SimulationTime& GetSimulationTime() const;

    World& GetWorld();
    Window& GetWindow();
    Renderer& GetRenderer();
    ResourceManager& GetResources();
    DebugUI& GetDebugUI();

private:
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer,
           const EngineDesc& desc);

    // Destroyed in reverse order:
    // the world (and its systems) first, then the resources, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    ResourceManager m_resources;
    DebugUI m_debugUI;
    World m_world;
    bool m_quitRequested = false;
    std::chrono::steady_clock::duration m_tickDuration;
    SimulationTime m_simulationTime;
};

} // namespace ezse
