add_library(engine_ui
    src/Interface.cpp
    src/Layout.cpp
    src/UiButtonStyleSystem.cpp
    src/UiInputSystem.cpp
    src/UiRenderSystem.cpp
)
add_library(engine::ui ALIAS engine_ui)

target_include_directories(engine_ui PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_ui
    PUBLIC engine::core engine::render engine::input
)
