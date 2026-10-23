#pragma once

#include "EZSE/ECS/World.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

#include <memory>

namespace ezse {

class Engine {
public:
    static std::unique_ptr<Engine> Create(const WindowDesc& windowDesc);

    void Run();
    void Quit();

    World& GetWorld();
    Window& GetWindow();
    Renderer& GetRenderer();

private:
    Engine(std::unique_ptr<Window> window, std::unique_ptr<Renderer> renderer);

    // Destroyed in reverse order:
    // the world (and its systems) first, the window last.
    std::unique_ptr<Window> m_window;
    std::unique_ptr<Renderer> m_renderer;
    World m_world;
    bool m_quitRequested = false;
};

} // namespace ezse
