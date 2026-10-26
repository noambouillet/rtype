add_library(engine_render
    src/RenderSystem.cpp
)
add_library(engine::render ALIAS engine_render)

target_include_directories(engine_render PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_render
    PUBLIC engine::core
    PRIVATE SFML::Graphics SFML::Window SFML::System
)
