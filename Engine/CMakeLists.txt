# Copies the folder <directory> next to the executable of <target>, into assets/<namespace>, on
# every build, so a changed file is there after the next build even if the executable did not
# relink. On disk this is the same tree as the virtual file system: the program mounts
# GetExecutableDirectory() / "assets" / "<namespace>" as "<namespace>". A file removed from
# <directory> stays next to the executable until the build folder is cleaned.
function(ezse_copy_assets target namespace directory)
    set(assetsTarget ${target}Assets_${namespace})
    # Not $<TARGET_FILE_DIR:target>: that makes this target depend on the executable, which itself
    # depends on this target, a cycle (decision 150). The folder of the target plus the config is
    # where Visual Studio puts the executable.
    get_target_property(binaryDir ${target} BINARY_DIR)
    add_custom_target(${assetsTarget} ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${directory}
            ${binaryDir}/$<CONFIG>/assets/${namespace}
        COMMENT "Copying the ${namespace} assets of ${target}"
    )
    add_dependencies(${target} ${assetsTarget})
endfunction()

# The assets of the engine itself (shaders, the cube) as namespace "engine". Every executable that
# creates an Engine needs them: Engine::Create fails if it cannot mount them.
function(ezse_copy_engine_assets target)
    # The folder of this file, also when called from the game (CMAKE_CURRENT_SOURCE_DIR would be
    # the folder of the caller).
    ezse_copy_assets(${target} engine ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/assets)
endfunction()

add_library(EZSEEngine STATIC
    # Not compiled; listed so they show in Visual Studio.
    assets/shaders/basic.frag
    assets/shaders/basic.vert
    assets/shaders/lit.frag
    assets/shaders/lit.vert
    assets/shaders/normals.frag
    assets/shaders/normals.vert
    assets/shaders/uv.frag
    assets/shaders/uv.vert
    include/EZSE/Engine/Components/Camera.h
    include/EZSE/Engine/Components/Light.h
    include/EZSE/Engine/Components/MeshRenderer.h
    include/EZSE/Engine/Components/PreviousTransform.h
    include/EZSE/Engine/Components/Transform.h
    include/EZSE/Engine/Components/WorldMatrix.h
    include/EZSE/Engine/Debug/ComponentEditors.h
    include/EZSE/Engine/Debug/DebugUI.h
    include/EZSE/Engine/Debug/DebugWidgets.h
    include/EZSE/Engine/Engine.h
    include/EZSE/Engine/InputState.h
    include/EZSE/Engine/Resources/Image.h
    include/EZSE/Engine/Resources/MeshData.h
    include/EZSE/Engine/Resources/Obj.h
    include/EZSE/Engine/Resources/ResourceCache.h
    include/EZSE/Engine/Resources/ResourceManager.h
    include/EZSE/Engine/SimulationTime.h
    include/EZSE/Engine/Systems/CameraSystem.h
    include/EZSE/Engine/Systems/LightSystem.h
    include/EZSE/Engine/Systems/RenderSystem.h
    include/EZSE/Engine/Systems/TransformHistorySystem.h
    include/EZSE/Engine/Systems/WorldMatrixSystem.h
    src/Components/PreviousTransform.cpp
    src/Components/Transform.cpp
    src/Components/WorldMatrix.cpp
    src/Debug/ComponentEditors.cpp
    src/Debug/DebugUI.cpp
    src/Debug/DebugWidgets.cpp
    src/Engine.cpp
    src/InputState.cpp
    src/Resources/Image.cpp
    src/Resources/Obj.cpp
    src/Resources/ResourceManager.cpp
    src/Systems/CameraSystem.cpp
    src/Systems/LightSystem.cpp
    src/Systems/RenderSystem.cpp
    src/Systems/TransformHistorySystem.cpp
    src/Systems/WorldMatrixSystem.cpp
)
add_library(EZSE::Engine ALIAS EZSEEngine)

target_include_directories(EZSEEngine PUBLIC include)
ezse_set_warnings(EZSEEngine)
target_link_libraries(EZSEEngine
    PUBLIC EZSE::Platform EZSE::Renderer EZSE::ECS EZSE::Core imgui
    PRIVATE stb
)

if(EZSE_BUILD_TESTS)
    add_executable(EngineTests tests/EngineTests.cpp)
    target_link_libraries(EngineTests PRIVATE EZSE::Engine)
    ezse_set_warnings(EngineTests)
    ezse_copy_engine_assets(EngineTests)
    add_test(NAME EngineTests COMMAND EngineTests)
endif()
