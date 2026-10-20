#pragma once

#include "EZSE/ECS/Entity.h"

#include <cstdint>
#include <span>
#include <vector>

namespace ezse {

// A set of entities as a sparse set: packed for fast iteration, with O(1) insert,
// remove and lookup. The order of the entities is not fixed (remove swaps with the last).
class EntitySet {
public:
    // Does nothing if the entity is already in the set.
    void Insert(Entity entity);
    // Does nothing if the entity is not in the set.
    void Remove(Entity entity);
    bool Contains(Entity entity) const;
    // Valid until the next Insert or Remove.
    std::span<const Entity> GetEntities() const;

private:
    std::vector<Entity> m_entities;
    std::vector<std::uint32_t> m_sparse; // index: id - 1, value: place plus one, 0 = not present
};

} // namespace ezse
