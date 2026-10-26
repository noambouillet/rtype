add_library(engine_input
    src/InputMap.cpp
    src/sfml/SfmlInput.cpp
)
add_library(engine::input ALIAS engine_input)

target_include_directories(engine_input PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_input
    PUBLIC engine::core
    PRIVATE SFML::Window SFML::System
)
