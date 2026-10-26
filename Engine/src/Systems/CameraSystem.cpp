#include "EZSE/Engine/Systems/CameraSystem.h"

#include "EZSE/Engine/Components/Camera.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/ECS/World.h"
#include "EZSE/Math/Transform.h"

namespace ezse {

CameraSystem::CameraSystem(Window& window, Renderer& renderer)
    : m_window(window), m_renderer(renderer) {}

void CameraSystem::Update(World& world, float) {
    // The first camera is used.
    if (GetEntities().empty()) {
        return;
    }
    const Entity cameraEntity = GetEntities().front();

    // A minimized window has size zero; then the camera is left as it was.
    const FramebufferSize framebufferSize = m_window.GetFramebufferSize();
    if (framebufferSize.width == 0 || framebufferSize.height == 0) {
        return;
    }

    const float aspect = static_cast<float>(framebufferSize.width) /
                         static_cast<float>(framebufferSize.height);
    const Camera& camera = world.GetComponent<Camera>(cameraEntity);
    const Transform& transform = world.GetComponent<Transform>(cameraEntity);
    m_renderer.SetCamera(
        ViewMatrix(transform),
        Perspective(camera.fovY, aspect, camera.nearPlane, camera.farPlane),
        transform.position);
}

} // namespace ezse
