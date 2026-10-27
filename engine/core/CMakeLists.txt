add_library(engine_core
    src/ConfigParser.cpp
    src/EngineSettings.cpp
    src/Logger.cpp
    src/ecs/ComponentManager.cpp
    src/ecs/Coordinator.cpp
    src/ecs/Entity.cpp
    src/ecs/EntityManager.cpp
    src/ecs/SystemManager.cpp
    src/systems/DestroySystem.cpp
    src/systems/MovementSystem.cpp
)
add_library(engine::core ALIAS engine_core)

target_include_directories(engine_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
