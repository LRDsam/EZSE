#include "EZSE/ECS/ComponentManager.h"

namespace ezse {

void ComponentManager::EntityDestroyed(Entity entity) {
    for (const auto& array : m_arrays) {
        if (array != nullptr) {
            array->EntityDestroyed(entity);
        }
    }
}

} // namespace ezse
