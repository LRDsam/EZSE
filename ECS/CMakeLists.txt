add_library(EZSEECS STATIC
    include/EZSE/ECS/ComponentArray.h
    include/EZSE/ECS/ComponentManager.h
    include/EZSE/ECS/ComponentType.h
    include/EZSE/ECS/Entity.h
    include/EZSE/ECS/EntityManager.h
    include/EZSE/ECS/EntitySet.h
    include/EZSE/ECS/Signature.h
    src/ComponentManager.cpp
    src/EntityManager.cpp
    src/EntitySet.cpp
)
add_library(EZSE::ECS ALIAS EZSEECS)

target_include_directories(EZSEECS PUBLIC include)
target_link_libraries(EZSEECS PUBLIC EZSE::Core)

add_executable(ECSTests tests/ECSTests.cpp)
target_link_libraries(ECSTests PRIVATE EZSE::ECS)
