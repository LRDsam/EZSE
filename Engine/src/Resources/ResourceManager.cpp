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
    for (MeshHandle mesh : m_meshes.GetHandles()) {
        m_renderer.DestroyMesh(mesh);
    }
}

MeshHandle ResourceManager::LoadMesh(const std::filesystem::path& path) {
    const std::string key = path.lexically_normal().generic_string();
    MeshHandle mesh = m_meshes.Acquire(key);
    if (mesh.id != 0) {
        return mesh;
    }
    std::optional<MeshData> meshData = ReadObjFile(m_root / path);
    if (!meshData.has_value()) {
        return MeshHandle{};
    }
    mesh = m_renderer.CreateMesh(meshData->vertices, meshData->indices);
    m_meshes.Add(key, mesh);
    LogInfo("Loaded mesh '{}' (id {})", key, mesh.id);
    return mesh;
}

void ResourceManager::UnloadMesh(MeshHandle mesh) {
    const std::string key = m_meshes.GetKey(mesh);
    if (m_meshes.Release(mesh)) {
        LogInfo("Unloaded mesh '{}' (id {})", key, mesh.id);
        m_renderer.DestroyMesh(mesh);
    }
}

} // namespace ezse
