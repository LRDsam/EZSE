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

# Dear ImGui: the debug UI. It has no CMake of its own, so the targets are made here.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.93.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
)
target_include_directories(imgui PUBLIC ${imgui_SOURCE_DIR})

# The two backends, each used by the module that owns that API: GLFW input in Platform,
# OpenGL drawing in Renderer (rules 1 and 2).
add_library(imgui_glfw STATIC ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp)
target_include_directories(imgui_glfw PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_glfw PUBLIC imgui PRIVATE glfw)

add_library(imgui_opengl3 STATIC ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp)
target_include_directories(imgui_opengl3 PUBLIC ${imgui_SOURCE_DIR}/backends)
target_link_libraries(imgui_opengl3 PUBLIC imgui)
