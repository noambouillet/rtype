add_library(engine_physics
    src/CollisionSystem.cpp
)
add_library(engine::physics ALIAS engine_physics)

target_include_directories(engine_physics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_physics
    PUBLIC engine::core
)
