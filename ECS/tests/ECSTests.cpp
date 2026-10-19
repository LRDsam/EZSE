#include "EZSE/ECS/EntityManager.h"

#include <iostream>

namespace {

int failureCount = 0;

void Check(bool condition, const char* description) {
    if (condition) {
        std::cout << "[PASS] " << description << '\n';
    } else {
        std::cout << "[FAIL] " << description << '\n';
        ++failureCount;
    }
}

void TestCreateEntity() {
    ezse::EntityManager entities;
    const ezse::Entity first = entities.CreateEntity();
    const ezse::Entity second = entities.CreateEntity();

    Check(first.id == 1, "CreateEntity: the first entity has id 1");
    Check(second.id == 2, "CreateEntity: the second entity has id 2");
    Check(entities.IsAlive(first), "CreateEntity: the first entity is alive");
    Check(entities.IsAlive(second), "CreateEntity: the second entity is alive");
}

void TestDestroyEntity() {
    ezse::EntityManager entities;
    const ezse::Entity first = entities.CreateEntity();
    const ezse::Entity second = entities.CreateEntity();
    entities.DestroyEntity(first);

    Check(!entities.IsAlive(first), "DestroyEntity: the destroyed entity is no longer alive");
    Check(entities.IsAlive(second), "DestroyEntity: the other entity is still alive");
}

void TestNoReuse() {
    ezse::EntityManager entities;
    const ezse::Entity first = entities.CreateEntity();
    entities.DestroyEntity(first);
    const ezse::Entity next = entities.CreateEntity();

    Check(next.id == 2, "No reuse: an entity after a destroy gets a new id");
    Check(!entities.IsAlive(first), "No reuse: the destroyed entity stays dead");
}

void TestInvalidEntity() {
    ezse::EntityManager entities;
    Check(!entities.IsAlive(ezse::Entity{}), "IsAlive: a default Entity is not alive");

    entities.CreateEntity();
    Check(!entities.IsAlive(ezse::Entity{0}), "IsAlive: id 0 is not alive");
    Check(!entities.IsAlive(ezse::Entity{2}), "IsAlive: an id that was never handed out is not alive");
}

} // namespace

int main() {
    TestCreateEntity();
    TestDestroyEntity();
    TestNoReuse();
    TestInvalidEntity();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
