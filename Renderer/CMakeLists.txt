add_library(EZSERenderer STATIC
    include/EZSE/Renderer/Mesh.h
    include/EZSE/Renderer/Material.h
    include/EZSE/Renderer/Renderer.h
    include/EZSE/Renderer/Shader.h
    include/EZSE/Renderer/Texture.h
    src/Renderer.cpp
)
add_library(EZSE::Renderer ALIAS EZSERenderer)

target_include_directories(EZSERenderer PUBLIC include)
target_link_libraries(EZSERenderer
    PUBLIC EZSE::Math
    PRIVATE EZSE::Core glad
)
