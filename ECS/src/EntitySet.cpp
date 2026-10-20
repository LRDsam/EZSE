#include "EZSE/ECS/EntitySet.h"

#include "EZSE/Core/Assert.h"

namespace ezse {

void EntitySet::Insert(Entity entity) {
    EZSE_ASSERT(entity.id != 0, "Cannot insert entity 0");
    if (Contains(entity)) {
        return;
    }

    m_entities.push_back(entity);
    if (entity.id > m_sparse.size()) {
        m_sparse.resize(entity.id);
    }
    m_sparse[entity.id - 1] = static_cast<std::uint32_t>(m_entities.size());
}

void EntitySet::Remove(Entity entity) {
    if (!Contains(entity)) {
        return;
    }

    const std::uint32_t position = m_sparse[entity.id - 1] - 1;
    m_entities[position] = m_entities.back();

    m_sparse[m_entities.back().id - 1] = m_sparse[entity.id - 1];
    m_sparse[entity.id - 1] = 0;

    m_entities.pop_back();
}

bool EntitySet::Contains(Entity entity) const {
    return entity.id != 0 &&
        entity.id <= m_sparse.size() &&
        m_sparse[entity.id - 1] != 0;
}

std::span<const Entity> EntitySet::GetEntities() const {
    return m_entities;
}

} // namespace ezse
