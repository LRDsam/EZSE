#pragma once

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Renderer.h"

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_map>

namespace ezse {

class ResourceManager {
public:
    ResourceManager(Renderer& renderer, std::filesystem::path root);
    ~ResourceManager();

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    MeshHandle LoadMesh(const std::filesystem::path& path);
    void UnloadMesh(MeshHandle mesh);

private:
    struct MeshEntry {
        std::string key;
        std::uint32_t count = 0;
    };

    Renderer& m_renderer;
    std::filesystem::path m_root;
    std::unordered_map<std::string, MeshHandle> m_meshesByKey;
    std::unordered_map<std::uint32_t, MeshEntry> m_meshEntries;
};

} // namespace ezse
