cmake_minimum_required(VERSION 3.15)
project(R-TypeWorkspace VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

add_subdirectory(engine)
add_subdirectory(games/rtype)
