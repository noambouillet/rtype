add_library(engine_audio
    src/AudioSystem.cpp
)
add_library(engine::audio ALIAS engine_audio)

target_include_directories(engine_audio PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_audio
    PUBLIC engine::core
    PRIVATE SFML::Audio SFML::System
)
