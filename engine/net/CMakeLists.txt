add_library(engine_net
    src/UdpSocket.cpp
    src/Packet.cpp
    src/Serializer.cpp
)
add_library(engine::net ALIAS engine_net)

target_include_directories(engine_net PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(engine_net
    PUBLIC engine::core
    PRIVATE asio::asio Threads::Threads
)

if(WIN32)
    target_compile_definitions(engine_net PRIVATE _WIN32_WINNT=0x0601 NOMINMAX)
    target_link_libraries(engine_net PRIVATE ws2_32)
endif()
