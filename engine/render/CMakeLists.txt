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
)

add_library(engine_render_sfml
    src/sfml/SfmlRenderer.cpp
)
add_library(engine::render_sfml ALIAS engine_render_sfml)

target_include_directories(engine_render_sfml PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_render_sfml
    PUBLIC engine::render
    PRIVATE SFML::Graphics SFML::Window SFML::System
)
