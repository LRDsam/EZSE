add_library(EZSEPhysics STATIC
    include/EZSE/Physics/Ray.h
    include/EZSE/Physics/Raycast.h
    include/EZSE/Physics/Shapes.h
    src/Raycast.cpp
)
add_library(EZSE::Physics ALIAS EZSEPhysics)

target_include_directories(EZSEPhysics PUBLIC include)
ezse_set_warnings(EZSEPhysics)
# Math for the vectors; ECS only for Entity, so a hit can say what it hit.
target_link_libraries(EZSEPhysics PUBLIC EZSE::Math EZSE::ECS)

if(EZSE_BUILD_TESTS)
    add_executable(PhysicsTests tests/PhysicsTests.cpp)
    target_link_libraries(PhysicsTests PRIVATE EZSE::Physics)
    ezse_set_warnings(PhysicsTests)
    add_test(NAME PhysicsTests COMMAND PhysicsTests)
endif()
