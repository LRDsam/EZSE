add_library(EZSEEngine STATIC
    include/EZSE/Engine/Engine.h
    src/Engine.cpp
)
add_library(EZSE::Engine ALIAS EZSEEngine)

target_include_directories(EZSEEngine PUBLIC include)
target_link_libraries(EZSEEngine
    PUBLIC EZSE::Platform EZSE::Renderer EZSE::ECS
    PRIVATE EZSE::Core
)
