#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Sends the view and projection of the first entity with a Transform and a Camera to the
// renderer, with the aspect taken from the window every frame.
class CameraSystem : public System {
public:
    CameraSystem(Window& window, Renderer& renderer);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Window& m_window;
    Renderer& m_renderer;
};

} // namespace ezse
