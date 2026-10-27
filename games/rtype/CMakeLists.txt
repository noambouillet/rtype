cmake_minimum_required(VERSION 3.15)
project(R-Type VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Werror)
endif()

option(RTYPE_BUILD_SERVER "Build r-type_server (engine core, physics, net)" ON)
option(RTYPE_BUILD_CLIENT "Build r-type_client (engine render, audio, input, net)" ON)

set(RTYPE_ENGINE_MODULES core net)
if(RTYPE_BUILD_SERVER)
    list(APPEND RTYPE_ENGINE_MODULES physics)
endif()
if(RTYPE_BUILD_CLIENT)
    list(APPEND RTYPE_ENGINE_MODULES render audio input)
endif()

if(NOT TARGET engine::core)
    find_package(Engine 0.1 CONFIG REQUIRED COMPONENTS ${RTYPE_ENGINE_MODULES})
endif()
foreach(RTYPE_ENGINE_MODULE IN LISTS RTYPE_ENGINE_MODULES)
    if(NOT TARGET engine::${RTYPE_ENGINE_MODULE})
        message(FATAL_ERROR "R-Type needs engine::${RTYPE_ENGINE_MODULE}: enable ENGINE_BUILD_<MODULE>")
    endif()
endforeach()

find_package(Threads REQUIRED)

set(RTYPE_ASSETS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets" CACHE PATH "R-Type assets directory")

add_library(rtype_protocol src/utils/network/Protocol.cpp)
target_include_directories(rtype_protocol PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(rtype_protocol PUBLIC engine::core)

if(RTYPE_BUILD_SERVER)
    set(RTYPE_GAME_SOURCES
        src/server/game/GameWorld.cpp
        src/server/game/WorldEvent.cpp
        src/server/game/WorldEventCollector.cpp
        src/server/game/Prefabs.cpp
        src/server/ecs/Systems/DamageSystem.cpp
        src/server/ecs/Systems/EnemySpawnSystem.cpp
        src/server/ecs/Systems/FormationSystem.cpp
        src/server/ecs/Systems/HitSystem.cpp
        src/server/ecs/Systems/HoverMotionSystem.cpp
        src/server/ecs/Systems/OffscreenSystem.cpp
        src/server/ecs/Systems/PlayerControlSystem.cpp
        src/server/ecs/Systems/ReplicationSystem.cpp
        src/server/ecs/Systems/SineMotionSystem.cpp
        src/server/ecs/Systems/WeaponSystem.cpp
    )

    add_executable(r-type_server
        src/server/main.cpp
        src/server/GameServer.cpp
        src/server/Session.cpp
        src/server/network/ServerNetworkService.cpp
        ${RTYPE_GAME_SOURCES}
    )

    target_link_libraries(r-type_server PRIVATE rtype_protocol engine::core engine::physics engine::net Threads::Threads)
    if(WIN32)
        target_link_libraries(r-type_server PRIVATE bcrypt)
    endif()
endif()

if(RTYPE_BUILD_CLIENT)
    add_library(rtype_client_core
        src/client/network/GameClient.cpp
        src/client/view/SpriteCatalog.cpp
        src/client/view/ClientNetworkSystem.cpp
        src/client/view/EffectSystem.cpp
        src/client/view/SoundEventSystem.cpp
        src/client/view/ClientWorld.cpp
        src/client/view/LocalFirePredictor.cpp
    )
    target_link_libraries(rtype_client_core PUBLIC engine::core engine::render engine::audio engine::input engine::net rtype_protocol)

    add_executable(r-type_client
        src/client/main.cpp
        src/client/app/ClientApp.cpp
        src/client/app/ClientControls.cpp
        src/client/app/ClientOptions.cpp
    )
    target_link_libraries(r-type_client PRIVATE rtype_client_core Threads::Threads)

    add_custom_target(rtype_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RTYPE_ASSETS_DIR}" "$<TARGET_FILE_DIR:r-type_client>/assets"
        DEPENDS r-type_client)
endif()
