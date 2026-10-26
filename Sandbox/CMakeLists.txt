add_executable(Sandbox
    src/main.cpp
    shaders/basic.vert
    shaders/basic.frag
    shaders/lit.vert
    shaders/lit.frag
    shaders/normals.vert
    shaders/normals.frag
)

target_link_libraries(Sandbox PRIVATE EZSE::Engine)

# Development only: the shaders are read from the source folder, so a changed shader is used
# without rebuilding. This path only exists on this computer (see the known limitations).
target_compile_definitions(Sandbox PRIVATE
    EZSE_SANDBOX_SHADER_DIR="${CMAKE_CURRENT_SOURCE_DIR}/shaders"
)
