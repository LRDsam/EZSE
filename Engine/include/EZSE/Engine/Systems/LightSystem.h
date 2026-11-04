#pragma once

#include "EZSE/ECS/System.h"
#include "EZSE/Renderer/Renderer.h"

namespace ezse {

// Sends the first directional light and the first ambient light to the renderer every frame,
// black when there is none. Point and spot lights wait until the renderer supports a list.
class LightSystem : public System {
public:
    explicit LightSystem(Renderer& renderer);

protected:
    void Update(World& world, float deltaTime) override;

private:
    Renderer& m_renderer;
};

} // namespace ezse
