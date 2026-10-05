add_library(EZSEPlatform STATIC src/Window.cpp)
add_library(EZSE::Platform ALIAS EZSEPlatform)

target_include_directories(EZSEPlatform PUBLIC include)
target_link_libraries(EZSEPlatform PRIVATE EZSE::Core glfw)
