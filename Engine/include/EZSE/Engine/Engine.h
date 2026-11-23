#pragma once

#include "EZSE/Core/VirtualFileSystem.h"

#include "EZSE/Engine/Debug/DebugDraw.h"
#include "EZSE/Engine/Debug/DebugUI.h"
#include "EZSE/Engine/InputState.h"
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

    // Runs the game loop until Quit, or until the window is closed: with a window the screen sets
    // the pace, headless the ticks do.
    void Run();

    // Stops Run after the current tick or frame. Can be called from a system.
    void Quit();

    // Runs exactly one tick of SystemPhase::Simulation and counts it, without looking at real time.
    // Run calls it for every tick; tests (and later bots and a server inside the client) call it
    // directly. Not from inside a system: the tick would run within itself.
    void Step();

    // Whether the Engine runs without a window (EngineDesc::window was empty).
    bool IsHeadless() const;

    // The tick number, the length of a tick and the blend between ticks.
    const SimulationTime& GetSimulationTime() const;
    // The input of this frame: for systems in SystemPhase::Presentation, such as looking around.
    // Empty when headless. Presses the debug UI wants (a click on a panel) are left out.
    const InputState& GetFrameInput() const;
    // The input since the previous tick: for systems in SystemPhase::Simulation, such as walking.
    // A press between two ticks is kept for the next one, and with two ticks in one frame only the
    // first sees it. Empty when headless.
    const InputState& GetTickInput() const;

    World& GetWorld();
    // The tree of virtual paths: mount at startup, before loading. Also there when running
    // headless, because the server reads data too.
    VirtualFileSystem& GetFileSystem();
    // Lines to see what the game does. Also there when running headless, where it draws nothing,
    // so the simulation can call it on a server too (decision 322).
    DebugDraw& GetDebugDraw();
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
    // The loop with a window: the screen sets the pace, each frame runs the ticks that are due.
    void RunWithWindow();
    // The loop without a window: the ticks set the pace, sleeping until the next one is due.
    void RunHeadless();

    // Destroyed in reverse order:
    // the world (and its systems) first, then the resources, then the file system they read
    // from, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    VirtualFileSystem m_fileSystem;
    // Empty when running headless: it uploads to the GPU through the Renderer.
    std::optional<ResourceManager> m_resources;
    DebugUI m_debugUI;
    // Enabled only with a window; declared after m_window, which the constructor reads for it.
    DebugDraw m_debugDraw;
    World m_world;
    bool m_quitRequested = false;
    std::chrono::steady_clock::duration m_tickDuration;
    SimulationTime m_simulationTime;
    // Filled each frame from the events of the window; see GetFrameInput and GetTickInput.
    InputState m_frameInput;
    // What happened since the previous tick; Step turns it into m_tickInput.
    InputState m_pendingTickInput;
    InputState m_tickInput;
};

} // namespace ezse
