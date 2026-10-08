add_executable(Sandbox src/main.cpp)

target_link_libraries(Sandbox PRIVATE EZSE::Core EZSE::Math EZSE::Platform EZSE::Renderer)
