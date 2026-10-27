include(FetchContent)

set(GLFW_BUILD_DOCS OFF)
set(GLFW_BUILD_TESTS OFF)
set(GLFW_BUILD_EXAMPLES OFF)
set(GLFW_INSTALL OFF)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glfw)

add_library(glad STATIC glad/src/gl.c)
target_include_directories(glad PUBLIC glad/include)

# stb_image: one header kept in git (see stb/README.md for the version). The code that uses it
# defines STB_IMAGE_IMPLEMENTATION in exactly one .cpp.
add_library(stb INTERFACE)
target_include_directories(stb INTERFACE stb)
