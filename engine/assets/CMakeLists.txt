add_library(engine_assets
    src/AssetManager.cpp
)
add_library(engine::assets ALIAS engine_assets)

target_include_directories(engine_assets PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_assets
    PUBLIC engine::core engine::render
    PRIVATE nlohmann_json::nlohmann_json
)
