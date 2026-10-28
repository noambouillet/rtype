add_library(engine_script
    src/ScriptEngine.cpp
    src/ScriptSystem.cpp
    src/LuaBindings.cpp
)
add_library(engine::script ALIAS engine_script)

target_include_directories(engine_script PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_script
    PUBLIC engine::core
    PRIVATE sol2::sol2 ${LUA_LIBRARIES}
)

target_include_directories(engine_script PRIVATE ${LUA_INCLUDE_DIR})
