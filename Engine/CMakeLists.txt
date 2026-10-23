add_library(EZSEEngine STATIC
    include/EZSE/Engine/Components/Camera.h
    include/EZSE/Engine/Components/MeshRenderer.h
    include/EZSE/Engine/Components/Transform.h
    include/EZSE/Engine/Engine.h
    src/Components/Transform.cpp
    src/Engine.cpp
)
add_library(EZSE::Engine ALIAS EZSEEngine)

target_include_directories(EZSEEngine PUBLIC include)
target_link_libraries(EZSEEngine
    PUBLIC EZSE::Platform EZSE::Renderer EZSE::ECS
    PRIVATE EZSE::Core
)

add_executable(EngineTests tests/EngineTests.cpp)
target_link_libraries(EngineTests PRIVATE EZSE::Engine)
