cmake_minimum_required(VERSION 3.20)

project(Engine)

set(CXX_STANDARD 18)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

include_directories(Engine include/)
set(ENGINE_SOURCES src/main.cpp)

add_executable(Engine ${ENGINE_SOURCES})

#Dependencies
add_subdirectory(deps/glfw)
target_link_libraries(Engine PUBLIC glfw)

add_subdirectory(deps/glad)
target_link_libraries(Engine PUBLIC glad)

set_target_properties(Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set_target_properties(Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR})
set_target_properties(Engine PROPERTIES RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR})