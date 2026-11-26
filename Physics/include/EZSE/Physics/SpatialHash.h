#pragma once

#include "EZSE/Math/Aabb.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace ezse {

// Finds what lies near a place: cubic cells of cellSize meters in a hash table, and every id in
// the cells its bounding box touches. It knows ids and their Aabb, nothing else: the precise
// test against a shape is up to the user. The ids come from the user: small and close together
// (0, 1, 2 and on, reused after Remove), because the hash keeps data per id in vectors indexed by
// the id.
class SpatialHash {
public:
    // cellSize in meters, above 0.
    explicit SpatialHash(float cellSize);

    // Adds id with the cells that bounds touches. Once per id until Remove (asserts).
    void Insert(std::uint32_t id, const Aabb& bounds);

    // New bounds for an id that was inserted. Only stores them if it touches the same cells.
    void Move(std::uint32_t id, const Aabb& bounds);

    // Takes id out of all its cells. The id may be inserted again afterwards.
    void Remove(std::uint32_t id);

    // Clears out and fills it with every id whose Aabb overlaps bounds (touching counts), each
    // once. The cells only make it fast: an id that shares a cell without overlapping is left out.
    void Query(const Aabb& bounds, std::vector<std::uint32_t>& out);

private:
    // A cell: floor(position / cellSize) on each axis.
    struct Cell {
        std::int32_t x = 0;
        std::int32_t y = 0;
        std::int32_t z = 0;

        bool operator==(const Cell&) const = default;
    };

    // The cells an Aabb touches: from the cell of min up to and including the cell of max.
    struct CellRange {
        Cell min;
        Cell max;

        bool operator==(const CellRange&) const = default;
    };

    // What the hash knows about one id.
    struct Entry {
        Aabb bounds;              // for the overlap test in Query
        CellRange cells;          // where the id is, so Move and Remove only touch these cells
        std::uint32_t stamp = 0;  // the last query that took this id; 0 is none
        bool inserted = false;    // false before Insert and after Remove
    };

    float m_cellSize = 0.0f;
    std::unordered_map<std::uint64_t, std::vector<std::uint32_t>> m_cells; // key: KeyOf(cell)
    std::vector<Entry> m_entries; // index: id
    std::uint32_t m_queryStamp = 0; // one higher for each query

    Cell CellOf(const Vec3& position) const;            // floor(position / m_cellSize) per axis
    CellRange CellsOf(const Aabb& bounds) const;        // CellOf(min) and CellOf(max)
    static std::uint64_t KeyOf(const Cell& cell);       // 21 bits per axis in one number
    void AddToCells(std::uint32_t id, const CellRange& cells);
    void RemoveFromCells(std::uint32_t id, const CellRange& cells);
};

} // namespace ezse
