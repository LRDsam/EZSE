cmake_minimum_required(VERSION 3.24)

project(EZSE VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# On when EZSE itself is built, off when a game includes it with add_subdirectory.
option(EZSE_BUILD_SANDBOX "Build the Sandbox test game" ${PROJECT_IS_TOP_LEVEL})
option(EZSE_BUILD_TESTS "Build the test programs" ${PROJECT_IS_TOP_LEVEL})

# Strict warnings for our own code, not for GLFW, glad, stb or Dear ImGui. CMake no longer adds
# /W3 by itself (since 3.15), so without this MSVC warns about very little. /permissive- makes
# MSVC follow the C++ standard more closely. A game can call it for its own targets too.
function(ezse_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endfunction()

# The test programs as CTest tests: after a build, "ctest --test-dir build -C Debug" runs them all.
if(EZSE_BUILD_TESTS)
    enable_testing()
endif()

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
