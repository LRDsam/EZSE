#include "EZSE/ECS/ComponentManager.h"

namespace ezse {

void ComponentManager::EntityDestroyed(Entity entity) {
    for (const auto& array : m_arrays) {
        if (array != nullptr) {
            array->EntityDestroyed(entity);
        }
    }
    // An entity that never had a component may have no signature yet.
    if (entity.id != 0 && entity.id <= m_signatures.size()) {
        m_signatures[entity.id - 1].reset();
    }
}

Signature ComponentManager::GetSignature(Entity entity) const {
    if (entity.id == 0 || entity.id > m_signatures.size()) {
        return Signature{};
    }
    return m_signatures[entity.id - 1];
}

} // namespace ezse
