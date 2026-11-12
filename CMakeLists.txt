cmake_minimum_required(VERSION 3.24)

project(EZSE VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On when EZSE itself is built, off when a game includes it with add_subdirectory.
option(EZSE_BUILD_SANDBOX "Build the Sandbox test game" ${PROJECT_IS_TOP_LEVEL})
option(EZSE_BUILD_TESTS "Build the test programs" ${PROJECT_IS_TOP_LEVEL})

add_subdirectory(ThirdParty)
add_subdirectory(Core)
add_subdirectory(Math)
add_subdirectory(ECS)
add_subdirectory(Platform)
add_subdirectory(Renderer)
add_subdirectory(Engine)
if(EZSE_BUILD_SANDBOX)
    add_subdirectory(Sandbox)
    set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT Sandbox)
endif()
