#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Platform/Window.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Sends the view and projection of the first entity with a WorldMatrix and a Camera to the
// renderer, with the aspect taken from the window every frame. The view is the inverse of the
// WorldMatrix.
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
