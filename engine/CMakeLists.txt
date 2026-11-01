cmake_minimum_required(VERSION 3.15)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(ENGINE_IS_TOP_LEVEL ON)
else()
    set(ENGINE_IS_TOP_LEVEL OFF)
endif()

include(CMakeDependentOption)

option(ENGINE_BUILD_RENDER "engine::render" ON)
cmake_dependent_option(ENGINE_BUILD_ASSETS "engine::assets" ON "ENGINE_BUILD_RENDER" OFF)
option(ENGINE_BUILD_AUDIO "engine::audio" ON)
option(ENGINE_BUILD_INPUT "engine::input" ON)
option(ENGINE_BUILD_PHYSICS "engine::physics" ON)
option(ENGINE_BUILD_NET "engine::net" ON)
option(ENGINE_BUILD_SCRIPT "engine::script (not implemented yet)" OFF)
option(ENGINE_BUILD_TOOLS "engine::tools, needs render and input" OFF)
option(ENGINE_WARNINGS_AS_ERRORS "Treat warnings as errors" ON)
option(ENGINE_INSTALL "Generate install and export rules" ${ENGINE_IS_TOP_LEVEL})

foreach(ENGINE_MODULE IN ITEMS script)
    string(TOUPPER ${ENGINE_MODULE} ENGINE_MODULE_OPTION)
    if(ENGINE_BUILD_${ENGINE_MODULE_OPTION})
        message(FATAL_ERROR "engine::${ENGINE_MODULE} is not implemented yet: set ENGINE_BUILD_${ENGINE_MODULE_OPTION}=OFF")
    endif()
endforeach()

if(ENGINE_BUILD_TOOLS AND NOT (ENGINE_BUILD_RENDER AND ENGINE_BUILD_INPUT))
    message(FATAL_ERROR "ENGINE_BUILD_TOOLS requires ENGINE_BUILD_RENDER and ENGINE_BUILD_INPUT")
endif()

set(ENGINE_MODULES core)
foreach(ENGINE_MODULE IN ITEMS render assets audio input physics net script tools)
    string(TOUPPER ${ENGINE_MODULE} ENGINE_MODULE_OPTION)
    if(ENGINE_BUILD_${ENGINE_MODULE_OPTION})
        list(APPEND ENGINE_MODULES ${ENGINE_MODULE})
    endif()
endforeach()

if(ENGINE_IS_TOP_LEVEL)
    set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON)
    set(VCPKG_MANIFEST_FEATURES ${ENGINE_MODULES})
    list(REMOVE_ITEM VCPKG_MANIFEST_FEATURES core physics)
endif()

project(Engine VERSION 0.1.0 LANGUAGES CXX)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(ENGINE_SFML_COMPONENTS "")
if(ENGINE_BUILD_RENDER)
    list(APPEND ENGINE_SFML_COMPONENTS Graphics Window System)
endif()
if(ENGINE_BUILD_ASSETS)
    find_package(nlohmann_json CONFIG REQUIRED)
endif()
if(ENGINE_BUILD_AUDIO)
    list(APPEND ENGINE_SFML_COMPONENTS Audio System)
endif()
if(ENGINE_BUILD_INPUT)
    list(APPEND ENGINE_SFML_COMPONENTS Window System)
endif()
list(REMOVE_DUPLICATES ENGINE_SFML_COMPONENTS)

if(ENGINE_SFML_COMPONENTS)
    find_package(SFML 3.0.2 EXACT COMPONENTS ${ENGINE_SFML_COMPONENTS} CONFIG REQUIRED)
endif()
if(ENGINE_BUILD_NET)
    find_package(asio CONFIG REQUIRED)
    find_package(Threads REQUIRED)
endif()
if(ENGINE_BUILD_SCRIPT)
    find_package(Lua REQUIRED)
    find_package(sol2 CONFIG REQUIRED)
endif()

foreach(ENGINE_MODULE IN LISTS ENGINE_MODULES)
    add_subdirectory(${ENGINE_MODULE})

    set_target_properties(engine_${ENGINE_MODULE} PROPERTIES
        EXPORT_NAME ${ENGINE_MODULE}
        CXX_EXTENSIONS OFF
    )
    target_compile_features(engine_${ENGINE_MODULE} PUBLIC cxx_std_20)
    if(MSVC)
        target_compile_options(engine_${ENGINE_MODULE} PRIVATE /W4 $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:/WX>)
    else()
        target_compile_options(engine_${ENGINE_MODULE} PRIVATE -Wall -Wextra $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:-Werror>)
    endif()
endforeach()

if(TARGET engine_render_sfml)
    set_target_properties(engine_render_sfml PROPERTIES
        EXPORT_NAME render_sfml
        CXX_EXTENSIONS OFF
    )
    target_compile_features(engine_render_sfml PUBLIC cxx_std_20)
    if(MSVC)
        target_compile_options(engine_render_sfml PRIVATE /W4 $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:/WX>)
    else()
        target_compile_options(engine_render_sfml PRIVATE -Wall -Wextra $<$<BOOL:${ENGINE_WARNINGS_AS_ERRORS}>:-Werror>)
    endif()
endif()

message(STATUS "Engine modules: ${ENGINE_MODULES}")

if(ENGINE_INSTALL)
    set(ENGINE_CMAKE_DIR ${CMAKE_INSTALL_LIBDIR}/cmake/Engine)

    foreach(ENGINE_MODULE IN LISTS ENGINE_MODULES)
        install(TARGETS engine_${ENGINE_MODULE}
            EXPORT EngineTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
        install(DIRECTORY ${ENGINE_MODULE}/include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    endforeach()

    if(TARGET engine_render_sfml)
        install(TARGETS engine_render_sfml
            EXPORT EngineTargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
            RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif()

    install(EXPORT EngineTargets
        NAMESPACE engine::
        DESTINATION ${ENGINE_CMAKE_DIR}
    )
    configure_package_config_file(cmake/EngineConfig.cmake.in
        ${PROJECT_BINARY_DIR}/EngineConfig.cmake
        INSTALL_DESTINATION ${ENGINE_CMAKE_DIR}
    )
    write_basic_package_version_file(${PROJECT_BINARY_DIR}/EngineConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    install(FILES
        ${PROJECT_BINARY_DIR}/EngineConfig.cmake
        ${PROJECT_BINARY_DIR}/EngineConfigVersion.cmake
        DESTINATION ${ENGINE_CMAKE_DIR}
    )
endif()
