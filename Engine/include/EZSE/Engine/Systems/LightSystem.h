#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Sends the first directional light, the first ambient light (black when there is none) and all
// point and spot lights to the renderer every frame.
class LightSystem : public System {
public:
    explicit LightSystem(Renderer& renderer);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Renderer& m_renderer;
};

} // namespace ezse
