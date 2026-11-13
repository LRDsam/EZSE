#pragma once

#include "EZSE/Engine/Debug/DebugUI.h"
#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/ECS/World.h"
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
};

class Engine {
public:
    // Registers the built-in components (Transform, MeshRenderer, Camera, Light)
    // in its World; the game registers only its own components.
    static std::unique_ptr<Engine> Create(const EngineDesc& desc);

    ~Engine();

    void Run();
    void Quit();

    World& GetWorld();
    Window& GetWindow();
    Renderer& GetRenderer();
    ResourceManager& GetResources();
    DebugUI& GetDebugUI();

private:
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer,
           std::chrono::steady_clock::duration tickDuration);

    // Destroyed in reverse order:
    // the world (and its systems) first, then the resources, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    ResourceManager m_resources;
    DebugUI m_debugUI;
    World m_world;
    bool m_quitRequested = false;
    std::chrono::steady_clock::duration m_tickDuration;
    std::uint64_t m_tickCount = 0; // Ticks run since the start; 64 bits last billions of years.
};

} // namespace ezse
