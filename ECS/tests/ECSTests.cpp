#include "EZSE/ECS/ComponentArray.h"
#include "EZSE/ECS/ComponentManager.h"
#include "EZSE/ECS/ComponentType.h"
#include "EZSE/ECS/EntityManager.h"
#include "EZSE/ECS/EntitySet.h"
#include "EZSE/ECS/Signature.h"
#include "EZSE/ECS/SystemManager.h"

#include <iostream>

namespace {

int failureCount = 0;

// A component that only exists for the tests.
struct Health {
    int value = 0;
};

// A second component type, to check that types get different numbers.
struct Speed {
    float value = 0.0f;
};

// Gets a type number but is never registered, so a ComponentManager has an empty slot for it.
struct Unregistered {};

// Registered after Unregistered got its number, so its slot lies beyond the empty one.
struct Armor {
    int value = 0;
};

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

// Entities 4, 2 and 5 get a component, in that order; 1 and 3 do not.
void FillComponents(ezse::ComponentArray<Health>& components) {
    components.Add(ezse::Entity{4}, Health{40});
    components.Add(ezse::Entity{2}, Health{20});
    components.Add(ezse::Entity{5}, Health{50});
}

void TestComponentAdd() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);

    Check(components.Has(ezse::Entity{4}), "ComponentArray Add: entity 4 has the component");
    Check(components.Has(ezse::Entity{2}), "ComponentArray Add: entity 2 has the component");
    Check(components.Has(ezse::Entity{5}), "ComponentArray Add: entity 5 has the component");
    Check(components.Get(ezse::Entity{4}).value == 40, "ComponentArray Add: entity 4 gets its own value");
    Check(components.Get(ezse::Entity{2}).value == 20, "ComponentArray Add: entity 2 gets its own value");
    Check(components.Get(ezse::Entity{5}).value == 50, "ComponentArray Add: entity 5 gets its own value");
}

void TestComponentHas() {
    ezse::ComponentArray<Health> components;
    Check(!components.Has(ezse::Entity{1}), "ComponentArray Has: nothing in an empty array");

    FillComponents(components);
    Check(!components.Has(ezse::Entity{1}), "ComponentArray Has: entity 1 has no component");
    Check(!components.Has(ezse::Entity{3}), "ComponentArray Has: entity 3, between two others, has no component");
    Check(!components.Has(ezse::Entity{7}), "ComponentArray Has: an id beyond the sparse array has no component");
    Check(!components.Has(ezse::Entity{}), "ComponentArray Has: entity 0 has no component");
}

void TestComponentGetReference() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);
    components.Get(ezse::Entity{2}).value = 25;

    Check(components.Get(ezse::Entity{2}).value == 25, "ComponentArray Get: a change through the reference is kept");
}

void TestComponentRemove() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);
    components.Remove(ezse::Entity{4});

    Check(!components.Has(ezse::Entity{4}), "ComponentArray Remove: entity 4 no longer has the component");
    Check(components.Get(ezse::Entity{5}).value == 50, "ComponentArray Remove: the moved entity 5 keeps its value");
    Check(components.Get(ezse::Entity{2}).value == 20, "ComponentArray Remove: entity 2 keeps its value");
}

void TestComponentRemoveLast() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);
    components.Remove(ezse::Entity{5});

    Check(!components.Has(ezse::Entity{5}), "ComponentArray Remove last: entity 5 no longer has the component");
    Check(components.Get(ezse::Entity{4}).value == 40, "ComponentArray Remove last: entity 4 keeps its value");
    Check(components.Get(ezse::Entity{2}).value == 20, "ComponentArray Remove last: entity 2 keeps its value");

    components.Remove(ezse::Entity{2});
    components.Remove(ezse::Entity{4});
    Check(!components.Has(ezse::Entity{4}), "ComponentArray Remove last: the only component can be removed");
}

void TestComponentAddAfterRemove() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);
    components.Remove(ezse::Entity{4});
    components.Add(ezse::Entity{4}, Health{44});

    Check(components.Get(ezse::Entity{4}).value == 44, "ComponentArray Add after Remove: entity 4 gets its new value");
    Check(components.Get(ezse::Entity{5}).value == 50, "ComponentArray Add after Remove: entity 5 keeps its value");
}

void TestComponentType() {
    Check(ezse::ComponentType<Health>() == ezse::ComponentType<Health>(),
          "ComponentType: the same type always gets the same number");
    Check(ezse::ComponentType<Health>() != ezse::ComponentType<Speed>(),
          "ComponentType: two types get different numbers");
}

void TestComponentEntityDestroyed() {
    ezse::ComponentArray<Health> components;
    FillComponents(components);
    ezse::IComponentArray& base = components;

    base.EntityDestroyed(ezse::Entity{4});
    Check(!components.Has(ezse::Entity{4}), "EntityDestroyed: the component of entity 4 is removed");
    Check(components.Get(ezse::Entity{5}).value == 50, "EntityDestroyed: the moved entity 5 keeps its value");

    base.EntityDestroyed(ezse::Entity{1});
    Check(components.Has(ezse::Entity{2}) && components.Has(ezse::Entity{5}),
          "EntityDestroyed: an entity without the component changes nothing");
}

void TestComponentManager() {
    ezse::ComponentManager components;
    components.Register<Health>();
    components.Register<Speed>();

    components.Add(ezse::Entity{1}, Health{10});
    components.Add(ezse::Entity{1}, Speed{2.5f});
    components.Add(ezse::Entity{2}, Health{20});

    // Has is const, so this also uses the const GetArray.
    const ezse::ComponentManager& constComponents = components;
    Check(constComponents.Has<Health>(ezse::Entity{1}), "ComponentManager: entity 1 has Health");
    Check(constComponents.Has<Speed>(ezse::Entity{1}), "ComponentManager: entity 1 has Speed");
    Check(!constComponents.Has<Speed>(ezse::Entity{2}), "ComponentManager: entity 2 has no Speed");
    Check(components.Get<Health>(ezse::Entity{2}).value == 20, "ComponentManager: Get finds the Health of entity 2");
    Check(components.Get<Speed>(ezse::Entity{1}).value == 2.5f, "ComponentManager: Get finds the Speed of entity 1");

    components.Remove<Health>(ezse::Entity{1});
    Check(!components.Has<Health>(ezse::Entity{1}), "ComponentManager: Remove takes away only Health");
    Check(components.Has<Speed>(ezse::Entity{1}), "ComponentManager: Remove leaves Speed of the same entity");
}

void TestComponentManagerEntityDestroyed() {
    ezse::ComponentType<Unregistered>();
    ezse::ComponentManager components;
    components.Register<Health>();
    components.Register<Armor>();
    Check(ezse::ComponentType<Unregistered>() < ezse::ComponentType<Armor>(),
          "ComponentManager EntityDestroyed: the manager has an empty slot before Armor");

    components.Add(ezse::Entity{1}, Health{10});
    components.Add(ezse::Entity{1}, Armor{5});
    components.Add(ezse::Entity{2}, Health{20});
    components.EntityDestroyed(ezse::Entity{1});

    Check(!components.Has<Health>(ezse::Entity{1}), "ComponentManager EntityDestroyed: Health of entity 1 is removed");
    Check(!components.Has<Armor>(ezse::Entity{1}), "ComponentManager EntityDestroyed: Armor of entity 1 is removed");
    Check(components.Get<Health>(ezse::Entity{2}).value == 20,
          "ComponentManager EntityDestroyed: entity 2 keeps its Health");
}

void TestSignature() {
    ezse::ComponentManager components;
    components.Register<Health>();
    components.Register<Speed>();
    const std::uint32_t health = ezse::ComponentType<Health>();
    const std::uint32_t speed = ezse::ComponentType<Speed>();

    Check(components.GetSignature(ezse::Entity{}).none(), "Signature: entity 0 has an empty signature");
    Check(components.GetSignature(ezse::Entity{1}).none(), "Signature: an entity without components has an empty signature");

    components.Add(ezse::Entity{1}, Health{10});
    components.Add(ezse::Entity{1}, Speed{2.5f});
    components.Add(ezse::Entity{2}, Health{20});

    const ezse::Signature first = components.GetSignature(ezse::Entity{1});
    Check(first.test(health) && first.test(speed) && first.count() == 2,
          "Signature: Add sets the bits of Health and Speed for entity 1");
    const ezse::Signature second = components.GetSignature(ezse::Entity{2});
    Check(second.test(health) && second.count() == 1, "Signature: Add sets only the Health bit for entity 2");
    Check(components.GetSignature(ezse::Entity{99}).none(), "Signature: an id beyond all entities has an empty signature");

    components.Remove<Speed>(ezse::Entity{1});
    const ezse::Signature afterRemove = components.GetSignature(ezse::Entity{1});
    Check(afterRemove.test(health) && !afterRemove.test(speed), "Signature: Remove clears only the Speed bit");

    components.EntityDestroyed(ezse::Entity{2});
    Check(components.GetSignature(ezse::Entity{2}).none(), "Signature: EntityDestroyed clears the whole signature");
    Check(components.GetSignature(ezse::Entity{1}).test(health), "Signature: EntityDestroyed leaves other entities alone");

    components.EntityDestroyed(ezse::Entity{50});
    Check(components.GetSignature(ezse::Entity{50}).none(),
          "Signature: EntityDestroyed on an entity that never had a component is harmless");
}

void TestEntitySetInsert() {
    ezse::EntitySet set;
    Check(set.GetEntities().empty(), "EntitySet: a new set is empty");
    Check(!set.Contains(ezse::Entity{}), "EntitySet: entity 0 is never in the set");

    set.Insert(ezse::Entity{4});
    set.Insert(ezse::Entity{2});
    set.Insert(ezse::Entity{4});

    Check(set.Contains(ezse::Entity{4}) && set.Contains(ezse::Entity{2}), "EntitySet Insert: both entities are in the set");
    Check(!set.Contains(ezse::Entity{3}), "EntitySet Insert: an entity between them is not in the set");
    Check(!set.Contains(ezse::Entity{9}), "EntitySet Insert: an id beyond the sparse array is not in the set");
    Check(set.GetEntities().size() == 2, "EntitySet Insert: inserting an entity twice keeps it once");
}

void TestEntitySetRemove() {
    ezse::EntitySet set;
    set.Insert(ezse::Entity{4});
    set.Insert(ezse::Entity{2});
    set.Insert(ezse::Entity{5});

    set.Remove(ezse::Entity{4});
    Check(!set.Contains(ezse::Entity{4}), "EntitySet Remove: entity 4 is no longer in the set");
    Check(set.Contains(ezse::Entity{5}) && set.Contains(ezse::Entity{2}),
          "EntitySet Remove: the moved entity 5 and entity 2 are still in the set");

    set.Remove(ezse::Entity{4});
    set.Remove(ezse::Entity{7});
    Check(set.GetEntities().size() == 2, "EntitySet Remove: removing an absent entity changes nothing");

    set.Remove(ezse::Entity{2});
    set.Remove(ezse::Entity{5});
    Check(set.GetEntities().empty(), "EntitySet Remove: the set can be emptied, also from the back");
}

void TestEntitySetGetEntities() {
    ezse::EntitySet set;
    set.Insert(ezse::Entity{3});
    set.Insert(ezse::Entity{1});

    std::uint32_t idSum = 0;
    for (ezse::Entity entity : set.GetEntities()) {
        idSum += entity.id;
    }
    Check(idSum == 4, "EntitySet GetEntities: a loop visits every entity once");
}

void TestMakeSignature() {
    const std::uint32_t health = ezse::ComponentType<Health>();
    const std::uint32_t speed = ezse::ComponentType<Speed>();

    const ezse::Signature both = ezse::MakeSignature<Health, Speed>();
    Check(both.test(health) && both.test(speed) && both.count() == 2,
          "MakeSignature: two types set exactly their two bits");
    const ezse::Signature one = ezse::MakeSignature<Speed>();
    Check(one.test(speed) && one.count() == 1, "MakeSignature: one type sets one bit");
    Check(ezse::MakeSignature<>().none(), "MakeSignature: no types gives an empty signature");
}

// A system that only exists for the tests. It keeps a reference, to check that Add
// passes constructor arguments on as references.
class CountingSystem : public ezse::System {
public:
    explicit CountingSystem(int& counter) : counter(counter) {}
    int& counter;
};

// A system without constructor arguments.
class PlainSystem : public ezse::System {};

bool SystemHas(const ezse::System& system, ezse::Entity entity) {
    for (ezse::Entity member : system.GetEntities()) {
        if (member.id == entity.id) {
            return true;
        }
    }
    return false;
}

void TestSystemManagerAdd() {
    ezse::SystemManager systems;
    int counter = 0;
    CountingSystem& counting = systems.Add<CountingSystem>(ezse::MakeSignature<Health>(), counter);
    counter = 7;
    Check(counting.counter == 7, "SystemManager Add: a reference argument stays a reference");

    for (int i = 0; i < 20; ++i) {
        systems.Add<PlainSystem>(ezse::MakeSignature<Speed>());
    }
    counter = 8;
    Check(counting.counter == 8, "SystemManager Add: the returned reference stays valid as systems are added");
    Check(counting.GetEntities().empty(), "SystemManager Add: a new system has no entities");
}

void TestSystemManagerSignatureChanged() {
    ezse::SystemManager systems;
    PlainSystem& healthSystem = systems.Add<PlainSystem>(ezse::MakeSignature<Health>());
    PlainSystem& movingSystem = systems.Add<PlainSystem>(ezse::MakeSignature<Health, Speed>());

    systems.EntitySignatureChanged(ezse::Entity{1}, ezse::MakeSignature<Health, Speed>());
    systems.EntitySignatureChanged(ezse::Entity{2}, ezse::MakeSignature<Health>());
    systems.EntitySignatureChanged(ezse::Entity{3}, ezse::MakeSignature<Speed>());

    Check(SystemHas(healthSystem, ezse::Entity{1}),
          "SystemManager: an entity with more components than the system needs belongs to it");
    Check(SystemHas(healthSystem, ezse::Entity{2}),
          "SystemManager: an entity with exactly the components belongs to the system");
    Check(!SystemHas(healthSystem, ezse::Entity{3}),
          "SystemManager: an entity without a needed component does not belong to the system");
    Check(SystemHas(movingSystem, ezse::Entity{1}) && movingSystem.GetEntities().size() == 1,
          "SystemManager: only entity 1 has both components of the second system");

    systems.EntitySignatureChanged(ezse::Entity{1}, ezse::MakeSignature<Health>());
    Check(!SystemHas(movingSystem, ezse::Entity{1}),
          "SystemManager: an entity that loses a component leaves the system");
    Check(SystemHas(healthSystem, ezse::Entity{1}),
          "SystemManager: the same entity stays in a system it still matches");
}

void TestSystemManagerEntityDestroyed() {
    ezse::SystemManager systems;
    PlainSystem& healthSystem = systems.Add<PlainSystem>(ezse::MakeSignature<Health>());
    PlainSystem& movingSystem = systems.Add<PlainSystem>(ezse::MakeSignature<Health, Speed>());
    systems.EntitySignatureChanged(ezse::Entity{1}, ezse::MakeSignature<Health, Speed>());
    systems.EntitySignatureChanged(ezse::Entity{2}, ezse::MakeSignature<Health>());

    systems.EntityDestroyed(ezse::Entity{1});
    Check(!SystemHas(healthSystem, ezse::Entity{1}) && !SystemHas(movingSystem, ezse::Entity{1}),
          "SystemManager EntityDestroyed: the entity leaves every system");
    Check(SystemHas(healthSystem, ezse::Entity{2}), "SystemManager EntityDestroyed: other entities stay");
}

} // namespace

int main() {
    TestCreateEntity();
    TestDestroyEntity();
    TestNoReuse();
    TestInvalidEntity();
    TestComponentAdd();
    TestComponentHas();
    TestComponentGetReference();
    TestComponentRemove();
    TestComponentRemoveLast();
    TestComponentAddAfterRemove();
    TestComponentType();
    TestComponentEntityDestroyed();
    TestComponentManager();
    TestComponentManagerEntityDestroyed();
    TestSignature();
    TestEntitySetInsert();
    TestEntitySetRemove();
    TestEntitySetGetEntities();
    TestMakeSignature();
    TestSystemManagerAdd();
    TestSystemManagerSignatureChanged();
    TestSystemManagerEntityDestroyed();

    std::cout << failureCount << " check(s) failed\n";
    return failureCount == 0 ? 0 : 1;
}
