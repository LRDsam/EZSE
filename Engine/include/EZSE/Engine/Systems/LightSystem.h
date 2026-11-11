#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Sends the first directional light, the first ambient light (black when there is none) and all
// point and spot lights to the renderer every frame. The directional light casts shadows inside a
// cube around the origin; shadowExtent is half the size of that cube.
class LightSystem : public System {
public:
    LightSystem(Renderer& renderer, float shadowExtent);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Renderer& m_renderer;
    float m_shadowExtent = 0.0f;
};

} // namespace ezse
