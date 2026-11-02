#include "EZSE/Engine/Resources/ResourceManager.h"

#include "EZSE/Engine/Resources/Obj.h"

#include "EZSE/Core/Assert.h"
#include "EZSE/Core/Log.h"

#include <optional>
#include <utility>

namespace ezse {

ResourceManager::ResourceManager(Renderer& renderer, std::filesystem::path root) :
    m_renderer(renderer), m_root(std::move(root)) {}

ResourceManager::~ResourceManager() {
    for (const auto& [key, mesh] : m_meshesByKey) {
        m_renderer.DestroyMesh(mesh);
    }
}

MeshHandle ResourceManager::LoadMesh(const std::filesystem::path& path) {
    std::string key = path.lexically_normal().generic_string();
    auto it = m_meshesByKey.find(key);
    if (it != m_meshesByKey.end()) {
        auto meshEntry = m_meshEntries.find(it->second.id);
        EZSE_ASSERT(meshEntry != m_meshEntries.end(),
            "Mesh Entry Not found");
        meshEntry->second.count++;
        return it->second;
    }
    std::optional<MeshData> meshData = ReadObjFile(m_root / path);
    if (!meshData.has_value()) {
        return MeshHandle{};
    }
    MeshHandle mesh = m_renderer.CreateMesh(meshData->vertices, meshData->indices);
    m_meshesByKey.emplace(key, mesh);
    m_meshEntries.emplace(mesh.id, MeshEntry{key, 1});
    LogInfo("Loaded mesh '{}' (id {})", key, mesh.id);
    return mesh;
}

void ResourceManager::UnloadMesh(MeshHandle mesh) {
    auto it = m_meshEntries.find(mesh.id);
    EZSE_ASSERT(it != m_meshEntries.end(),
        "Mesh handle does not exist in mesh entries");
    if (--it->second.count == 0) {
        LogInfo("Unloaded mesh '{}' (id {})", it->second.key, mesh.id);
        m_meshesByKey.erase(it->second.key);
        m_meshEntries.erase(it);
        m_renderer.DestroyMesh(mesh);
    }
}

} // namespace ezse
