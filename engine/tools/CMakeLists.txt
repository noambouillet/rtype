add_library(engine_tools
    src/MetricsOverlay.cpp
)
add_library(engine::tools ALIAS engine_tools)

target_include_directories(engine_tools PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_tools
    PUBLIC engine::core engine::render engine::input
)
