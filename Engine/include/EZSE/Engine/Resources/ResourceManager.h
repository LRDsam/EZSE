#pragma once

#include "EZSE/Engine/Resources/ResourceCache.h"

#include "EZSE/Renderer/Mesh.h"
#include "EZSE/Renderer/Renderer.h"

#include <filesystem>
#include <string>

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
    Renderer& m_renderer;
    std::filesystem::path m_root;

    ResourceCache<MeshHandle> m_meshes;
};

} // namespace ezse
