cmake_minimum_required(VERSION 3.15)

option(RTYPE_BUILD_SERVER "Build r-type_server (engine core, physics, net)" ON)
option(RTYPE_BUILD_CLIENT "Build r-type_client (engine render, audio, input, net, tools)" ON)

set(ENGINE_BUILD_RENDER ${RTYPE_BUILD_CLIENT})
set(ENGINE_BUILD_AUDIO ${RTYPE_BUILD_CLIENT})
set(ENGINE_BUILD_INPUT ${RTYPE_BUILD_CLIENT})
set(ENGINE_BUILD_TOOLS ${RTYPE_BUILD_CLIENT})
set(ENGINE_BUILD_PHYSICS ${RTYPE_BUILD_SERVER})
set(ENGINE_BUILD_NET ON)

set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON)
set(VCPKG_MANIFEST_FEATURES "")
if(RTYPE_BUILD_SERVER)
    list(APPEND VCPKG_MANIFEST_FEATURES server)
endif()
if(RTYPE_BUILD_CLIENT)
    list(APPEND VCPKG_MANIFEST_FEATURES client)
endif()

project(R-TypeWorkspace VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(engine)
add_subdirectory(games/rtype)
