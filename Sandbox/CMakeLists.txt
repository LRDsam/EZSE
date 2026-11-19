add_executable(Sandbox src/main.cpp)

target_link_libraries(Sandbox PRIVATE EZSE::Engine)
ezse_set_warnings(Sandbox)

# The shaders and the cube come from the engine; the rest of Sandbox/assets is its own, as
# namespace "sandbox".
ezse_copy_engine_assets(Sandbox)
ezse_copy_assets(Sandbox sandbox ${CMAKE_CURRENT_SOURCE_DIR}/assets)
