add_library(EZSEEngine STATIC
    include/EZSE/Engine/Components/Camera.h
    include/EZSE/Engine/Components/Light.h
    include/EZSE/Engine/Components/MeshRenderer.h
    include/EZSE/Engine/Components/PreviousTransform.h
    include/EZSE/Engine/Components/Transform.h
    include/EZSE/Engine/Debug/ComponentEditors.h
    include/EZSE/Engine/Debug/DebugUI.h
    include/EZSE/Engine/Debug/DebugWidgets.h
    include/EZSE/Engine/Engine.h
    include/EZSE/Engine/Resources/Image.h
    include/EZSE/Engine/Resources/MeshData.h
    include/EZSE/Engine/Resources/Obj.h
    include/EZSE/Engine/Resources/ResourceCache.h
    include/EZSE/Engine/Resources/ResourceManager.h
    include/EZSE/Engine/Systems/CameraSystem.h
    include/EZSE/Engine/Systems/LightSystem.h
    include/EZSE/Engine/Systems/RenderSystem.h
    include/EZSE/Engine/Systems/TransformHistorySystem.h
    src/Components/PreviousTransform.cpp
    src/Components/Transform.cpp
    src/Debug/ComponentEditors.cpp
    src/Debug/DebugUI.cpp
    src/Debug/DebugWidgets.cpp
    src/Engine.cpp
    src/Resources/Image.cpp
    src/Resources/Obj.cpp
    src/Resources/ResourceManager.cpp
    src/Systems/CameraSystem.cpp
    src/Systems/LightSystem.cpp
    src/Systems/RenderSystem.cpp
    src/Systems/TransformHistorySystem.cpp
)
add_library(EZSE::Engine ALIAS EZSEEngine)

target_include_directories(EZSEEngine PUBLIC include)
target_link_libraries(EZSEEngine
    PUBLIC EZSE::Platform EZSE::Renderer EZSE::ECS EZSE::Core imgui
    PRIVATE stb
)

if(EZSE_BUILD_TESTS)
    add_executable(EngineTests tests/EngineTests.cpp)
    target_link_libraries(EngineTests PRIVATE EZSE::Engine)
endif()
