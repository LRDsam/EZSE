#pragma once

#include <cstdint>
#include <utility>

namespace ezse {

// Resources: one object per type, owned by a World (EZSE decision 332). The World stores them
// without knowing their types, like components, so the ECS depends on nobody: not on Physics and
// not on the game.

// A number per resource type, like ComponentType but with its own counter, so resources are
// numbered 0, 1, 2... without gaps and the number can be an index. The same warning as for
// ComponentType: the numbers can differ between two programs, so never send or store them.
inline std::uint32_t NextResourceType() {
    static std::uint32_t next = 0;
    return next++;
}

template <typename T> std::uint32_t ResourceType() {
    static const std::uint32_t type = NextResourceType();
    return type;
}

// Lets the World own objects whose type the ECS does not know: it keeps IResource pointers, and
// the virtual destructor destroys the right T.
class IResource {
public:
    virtual ~IResource() = default;
};

// Holds the one object of type T, made from the arguments of World::AddResource.
template <typename T>
class Resource : public IResource {
public:
    template <typename... Args>
    explicit Resource(Args&&... args);

    T value;
};

template <typename T>
template <typename... Args>
Resource<T>::Resource(Args&&... args) : value(std::forward<Args>(args)...) {}

} // namespace ezse
