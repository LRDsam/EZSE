add_executable(Sandbox
    src/main.cpp
    assets/shaders/basic.vert
    assets/shaders/basic.frag
    assets/shaders/lit.vert
    assets/shaders/lit.frag
    assets/shaders/normals.vert
    assets/shaders/normals.frag
    assets/shaders/uv.vert
    assets/shaders/uv.frag
)

target_link_libraries(Sandbox PRIVATE EZSE::Engine)

# Copies Sandbox/assets next to the executable on every build, so a changed shader is used after
# the next build. Later this step can pack or optimize the assets before they are copied.
add_custom_target(SandboxAssets ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        ${CMAKE_CURRENT_BINARY_DIR}/$<CONFIG>/assets
    COMMENT "Copying Sandbox assets"
)
add_dependencies(Sandbox SandboxAssets)
