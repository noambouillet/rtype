cmake_minimum_required(VERSION 3.15)
project(Engine VERSION 0.1.0 LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(ENGINE_IS_TOP_LEVEL ON)
else()
    set(ENGINE_IS_TOP_LEVEL OFF)
endif()

option(ENGINE_WARNINGS_AS_ERRORS "Treat engine compiler warnings as errors" ON)
option(ENGINE_INSTALL "Generate the engine install and export rules" ${ENGINE_IS_TOP_LEVEL})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

add_library(engine_core
    src/core/ConfigParser.cpp
    src/core/EngineSettings.cpp
    src/core/EventBus.cpp
    src/core/Logger.cpp
    src/core/ecs/ComponentManager.cpp
    src/core/ecs/Coordinator.cpp
    src/core/ecs/Entity.cpp
    src/core/ecs/EntityManager.cpp
    src/core/ecs/SystemManager.cpp
    src/core/systems/AudioSystem.cpp
    src/core/systems/CollisionSystem.cpp
    src/core/systems/DestroySystem.cpp
    src/core/systems/MovementSystem.cpp
    src/core/systems/RenderSystem.cpp
)
add_library(engine::core ALIAS engine_core)

set_target_properties(engine_core PROPERTIES
    EXPORT_NAME core
    CXX_EXTENSIONS OFF
)
target_compile_features(engine_core PUBLIC cxx_std_20)
target_include_directories(engine_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(MSVC)
    target_compile_options(engine_core PRIVATE /W4 $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:/WX>)
else()
    target_compile_options(engine_core PRIVATE -Wall -Wextra $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:-Werror>)
endif()

if(ENGINE_INSTALL)
    set(ENGINE_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

    install(TARGETS engine_core
        EXPORT EngineTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    install(EXPORT EngineTargets
        NAMESPACE engine::
        DESTINATION ${ENGINE_CMAKE_DIR}
    )

    configure_package_config_file(cmake/EngineConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/EngineConfig.cmake
        INSTALL_DESTINATION ${ENGINE_CMAKE_DIR}
    )
    write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/EngineConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${CMAKE_CURRENT_BINARY_DIR}/EngineConfig.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/EngineConfigVersion.cmake
        DESTINATION ${ENGINE_CMAKE_DIR}
    )
endif()
