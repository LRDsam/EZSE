add_library(EZSERenderer STATIC
    include/EZSE/Renderer/Mesh.h
    include/EZSE/Renderer/Renderer.h
    src/Renderer.cpp
)
add_library(EZSE::Renderer ALIAS EZSERenderer)

target_include_directories(EZSERenderer PUBLIC include)
target_link_libraries(EZSERenderer PRIVATE EZSE::Core glad)
