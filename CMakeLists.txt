cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME Engine)
project(${PROJECT_NAME}
        VERSION 0.0.1.1
        DESCRIPTION "Engine"
        LANGUAGES CXX)

set(PROJECT_SOURCES src/main.cpp
                    src/window/window.cpp
                    src/renderer/renderer.cpp)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
include_directories(src/)

find_package(Vulkan REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE Vulkan::Vulkan)

include(FetchContent)

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG
)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glfw)
target_link_libraries(${PROJECT_NAME} PRIVATE glfw)

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        1.0.3
)
FetchContent_MakeAvailable(glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)