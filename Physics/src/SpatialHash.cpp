#include "EZSE/Physics/SpatialHash.h"

#include "EZSE/Core/Assert.h"

#include <algorithm>
#include <cmath>

namespace ezse {

namespace {

constexpr int c_axisBits = 21;                                              // 3 * 21 = 63 bits
constexpr std::uint64_t c_axisMask = (std::uint64_t{1} << c_axisBits) - 1;  // 21 ones: 0x1FFFFF

} // namespace

SpatialHash::SpatialHash(float cellSize) : m_cellSize(cellSize) {}

void SpatialHash::Insert(std::uint32_t id, const Aabb& bounds) {
    // Only ask for m_entries[id] when it exists: || skips the right side if the left is true.
    EZSE_ASSERT(id >= m_entries.size() || !m_entries[id].inserted, "Id is already inserted");
    // resize also shrinks, so only when id lies beyond the end.
    if (m_entries.size() <= id) {
        m_entries.resize(id + 1);
    }
    const CellRange cells = CellsOf(bounds);
    AddToCells(id, cells);
    // A fresh Entry, so stamp is 0 again: no query has seen this id since it came back.
    m_entries[id] = Entry{
        .bounds = bounds,
        .cells = cells,
        .inserted = true
    };
}

void SpatialHash::Move(std::uint32_t id, const Aabb& bounds) {
    EZSE_ASSERT(id < m_entries.size() && m_entries[id].inserted, "Id is not inserted");
    Entry& entry = m_entries[id];
    const CellRange newCells = CellsOf(bounds);
    if (newCells != entry.cells) {
        RemoveFromCells(id, entry.cells);
        AddToCells(id, newCells);
        entry.cells = newCells;
    }
    entry.bounds = bounds;
}

void SpatialHash::Remove(std::uint32_t id) {
    // && skips the right side if the left is false, so m_entries[id] is only read when it exists.
    EZSE_ASSERT(id < m_entries.size() && m_entries[id].inserted, "Id is not inserted");
    Entry& entry = m_entries[id];
    RemoveFromCells(id, entry.cells);
    entry.inserted = false;
}

void SpatialHash::Query(const Aabb& bounds, std::vector<std::uint32_t>& out) {
    out.clear();
    ++m_queryStamp;
    if (m_queryStamp == 0) {
        // The counter went round: an old stamp could now equal the new one, and 0 means "not
        // seen". Start again from a clean slate, once every few billion queries.
        for (Entry& entry : m_entries) {
            entry.stamp = 0;
        }
        m_queryStamp = 1;
    }

    const CellRange cells = CellsOf(bounds);
    for (std::int32_t x = cells.min.x; x <= cells.max.x; ++x) {
        for (std::int32_t y = cells.min.y; y <= cells.max.y; ++y) {
            for (std::int32_t z = cells.min.z; z <= cells.max.z; ++z) {
                // find, not operator[]: looking must not create empty cells.
                const auto cell = m_cells.find(KeyOf(Cell{x, y, z}));
                if (cell == m_cells.end()) {
                    continue;
                }
                for (const std::uint32_t id : cell->second) {
                    Entry& entry = m_entries[id];
                    // Stamped before the overlap test, so an id in several cells is tested once.
                    if (entry.stamp == m_queryStamp) {
                        continue;
                    }
                    entry.stamp = m_queryStamp;
                    if (Overlaps(entry.bounds, bounds)) {
                        out.push_back(id);
                    }
                }
            }
        }
    }
}

SpatialHash::Cell SpatialHash::CellOf(const Vec3& position) const {
    return Cell{
        .x = static_cast<std::int32_t>(std::floor(position.x / m_cellSize)),
        .y = static_cast<std::int32_t>(std::floor(position.y / m_cellSize)),
        .z = static_cast<std::int32_t>(std::floor(position.z / m_cellSize))
    };
}

SpatialHash::CellRange SpatialHash::CellsOf(const Aabb& bounds) const {
    return CellRange{
        .min = CellOf(bounds.min),
        .max = CellOf(bounds.max)
    };
}

std::uint64_t SpatialHash::KeyOf(const Cell& cell) {
    const std::uint64_t x = static_cast<std::uint32_t>(cell.x) & c_axisMask;
    const std::uint64_t y = static_cast<std::uint32_t>(cell.y) & c_axisMask;
    const std::uint64_t z = static_cast<std::uint32_t>(cell.z) & c_axisMask;
    return (x << (c_axisBits * 2)) | (y << c_axisBits) | z;
}

void SpatialHash::AddToCells(std::uint32_t id, const CellRange& cells) {
    for (std::int32_t x = cells.min.x; x <= cells.max.x; ++x) {
        for (std::int32_t y = cells.min.y; y <= cells.max.y; ++y) {
            for (std::int32_t z = cells.min.z; z <= cells.max.z; ++z) {
                const std::uint64_t key = KeyOf(Cell{x, y, z});
                m_cells[key].push_back(id);
            }
        }
    }
}

void SpatialHash::RemoveFromCells(std::uint32_t id, const CellRange& cells) {
    for (std::int32_t x = cells.min.x; x <= cells.max.x; ++x) {
        for (std::int32_t y = cells.min.y; y <= cells.max.y; ++y) {
            for (std::int32_t z = cells.min.z; z <= cells.max.z; ++z) {
                // find, not operator[]: AddToCells made this cell, so a missing one is a bug.
                const auto cell = m_cells.find(KeyOf(Cell{x, y, z}));
                EZSE_ASSERT(cell != m_cells.end(), "Cell does not exist");
                std::vector<std::uint32_t>& ids = cell->second;

                const auto position = std::find(ids.begin(), ids.end(), id);
                EZSE_ASSERT(position != ids.end(), "Id is not in its cell");
                // The order in a cell does not matter: move the last id into the gap instead of
                // shifting everything after it.
                *position = ids.back();
                ids.pop_back();

                // An empty cell goes: as players walk across the world, the map would otherwise
                // keep every cell that ever held something, and grow without end.
                if (ids.empty()) {
                    m_cells.erase(cell);
                }
            }
        }
    }
}

} // namespace ezse
