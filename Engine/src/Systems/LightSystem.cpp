#include "EZSE/Engine/Systems/LightSystem.h"

#include "EZSE/Engine/Components/Light.h"
#include "EZSE/Engine/Components/Transform.h"

#include "EZSE/ECS/World.h"
#include "EZSE/Math/Transform.h"

namespace ezse {

LightSystem::LightSystem(Renderer& renderer) : m_renderer(renderer) {}

void LightSystem::Update(World& world, float) {
    bool directionalFound = false;
    bool ambientFound = false;
    Vec3 directionalDirection = { 0.0f, -1.0f, 0.0f };
    Vec3 directionalColor;
    Vec3 ambientColor;

    for (const auto& entity : GetEntities()) {
        const Transform& transform = world.GetComponent<Transform>(entity);
        const Light& light = world.GetComponent<Light>(entity);
        switch (light.type) {
        case LightType::Directional:
            if (directionalFound == true) {
                break;
            }
            directionalDirection = TransformDirection(
                RotationY(transform.rotation.y) * RotationX(transform.rotation.x),
                Vec3{0.0f, 0.0f, -1.0f});
            directionalColor = light.color * light.intensity;
            directionalFound = true;
            break;
        case LightType::Ambient:
            if (ambientFound == true) {
                break;
            }
            ambientColor = light.color * light.intensity;
            ambientFound = true;
            break;
        case LightType::Point:
            // Point Light functionality not yet implemented
            break;
        case LightType::Spot:
            // Spot Light functionality not yet implemented
            break;
        }
    }
    m_renderer.SetDirectionalLight(directionalDirection, directionalColor);
    m_renderer.SetAmbientLight(ambientColor);
}

} // namespace ezse
