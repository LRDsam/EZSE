#pragma once

#include "EZSE/Core/Assert.h"

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace ezse {

template <typename Handle>
class ResourceCache {
public:
    Handle Acquire(const std::string& key);
    void Add(const std::string& key, Handle handle);
    bool Release(Handle handle);

    const std::string& GetKey(Handle handle) const;
    std::vector<Handle> GetHandles() const;

private:
    struct Entry {
        std::string key;
        std::uint32_t count = 0;
    };

    std::unordered_map<std::string, Handle> m_handlesByKey;
    std::unordered_map<std::uint32_t, Entry> m_entries;
};

template <typename Handle>
Handle ResourceCache<Handle>::Acquire(const std::string& key) {
    auto it = m_handlesByKey.find(key);
    if (it == m_handlesByKey.end()) {
        return Handle{};
    }
    auto entry = m_entries.find(it->second.id);
    EZSE_ASSERT(entry != m_entries.end(), "Handle Not found");
    entry->second.count++;
    return it->second;
}

template <typename Handle>
void ResourceCache<Handle>::Add(const std::string& key, Handle handle) {
    EZSE_ASSERT(m_handlesByKey.find(key) == m_handlesByKey.end(), "Key already exists");
    EZSE_ASSERT(handle.id != 0, "Handle has id of 0");
    m_handlesByKey.emplace(key, handle);
    m_entries.emplace(handle.id, Entry{key, 1});
}

template <typename Handle>
bool ResourceCache<Handle>::Release(Handle handle) {
    auto it = m_entries.find(handle.id);
    EZSE_ASSERT(it != m_entries.end(), "handle does not exist in entries");
    if (--it->second.count == 0) {
        m_handlesByKey.erase(it->second.key);
        m_entries.erase(it);
        return true;
    }
    return false;
}

template <typename Handle>
const std::string& ResourceCache<Handle>::GetKey(Handle handle) const {
    auto it = m_entries.find(handle.id);
    EZSE_ASSERT(it != m_entries.end(), "Handle does not exist");
    return it->second.key;
}

template <typename Handle>
std::vector<Handle> ResourceCache<Handle>::GetHandles() const {
    std::vector<Handle> handles;
    handles.reserve(m_handlesByKey.size());
    for (const auto& [key, handle] : m_handlesByKey) {
        handles.push_back(handle);
    }
    return handles;
}

} // namespace ezse
