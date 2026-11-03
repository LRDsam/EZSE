#pragma once

#include "EZSE/Engine/Debug/DebugUI.h"
#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/ECS/World.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

#include <memory>

namespace ezse {

class Engine {
public:
    // Registers the built-in components (Transform, MeshRenderer, Camera)
    // in its World; the game registers only its own components.
    static std::unique_ptr<Engine> Create(const WindowDesc& windowDesc);

    ~Engine();

    void Run();
    void Quit();

    World& GetWorld();
    Window& GetWindow();
    Renderer& GetRenderer();
    ResourceManager& GetResources();
    DebugUI& GetDebugUI();

private:
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer);

    // Destroyed in reverse order:
    // the world (and its systems) first, then the resources, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    ResourceManager m_resources;
    DebugUI m_debugUI;
    World m_world;
    bool m_quitRequested = false;
};

} // namespace ezse
