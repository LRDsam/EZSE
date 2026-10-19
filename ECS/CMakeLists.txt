add_library(EZSEECS STATIC
    include/EZSE/ECS/ComponentArray.h
    include/EZSE/ECS/Entity.h
    include/EZSE/ECS/EntityManager.h
    src/EntityManager.cpp
)
add_library(EZSE::ECS ALIAS EZSEECS)

target_include_directories(EZSEECS PUBLIC include)
target_link_libraries(EZSEECS PUBLIC EZSE::Core)

add_executable(ECSTests tests/ECSTests.cpp)
target_link_libraries(ECSTests PRIVATE EZSE::ECS)
