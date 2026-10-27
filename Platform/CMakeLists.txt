add_library(EZSEPlatform STATIC
    include/EZSE/Platform/FileSystem.h
    include/EZSE/Platform/Key.h
    include/EZSE/Platform/Window.h
    include/EZSE/Platform/WindowEvent.h
    src/FileSystem.cpp
    src/Window.cpp
)
add_library(EZSE::Platform ALIAS EZSEPlatform)

target_include_directories(EZSEPlatform PUBLIC include)
target_link_libraries(EZSEPlatform PRIVATE EZSE::Core glfw)
