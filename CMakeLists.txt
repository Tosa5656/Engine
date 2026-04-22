cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME Engine)
project(${PROJECT_NAME}
        VERSION 0.0.1.1
        DESCRIPTION "Engine"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(PROJECT_SOURCES src/main.cpp
                    src/window/window.cpp
                    src/renderer/renderer.cpp
                    src/vma/vma_impl.cpp
                    src/renderer/vulkan/instance.cpp
                    src/renderer/vulkan/device.cpp
                    src/renderer/vulkan/swapchain.cpp
                    src/renderer/vulkan/commandbuffer.cpp
                    src/renderer/vulkan/pipeline.cpp
                    src/renderer/vulkan/descriptors.cpp
                    src/renderer/vulkan/resources.cpp)
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

FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG        master
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

find_program(GLSLC glslc REQUIRED)

function(compile_shaders TARGET_NAME SHADER_DIR OUTPUT_DIR)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    file(GLOB SHADERS
            "${SHADER_DIR}/*.vert"
            "${SHADER_DIR}/*.frag"
            "${SHADER_DIR}/*.comp"
            "${SHADER_DIR}/*.geom"
            "${SHADER_DIR}/*.tesc"
            "${SHADER_DIR}/*.tese"
    )

    set(OUTPUT_FILES "")
    foreach(SHADER ${SHADERS})
        get_filename_component(NAME_WE ${SHADER} NAME_WE)
        set(OUTPUT_FILE "${OUTPUT_DIR}/${NAME_WE}.spv")

        add_custom_command(
                OUTPUT ${OUTPUT_FILE}
                COMMAND ${GLSLC} "${SHADER}" -o "${OUTPUT_FILE}"
                DEPENDS "${SHADER}"
                COMMENT "glslc ${NAME_WE} → ${NAME_WE}.spv"
                VERBATIM
        )
        list(APPEND OUTPUT_FILES ${OUTPUT_FILE})
    endforeach()

    add_custom_target(${TARGET_NAME}_shaders ALL DEPENDS ${OUTPUT_FILES})
    add_dependencies(${TARGET_NAME} ${TARGET_NAME}_shaders)
endfunction()

compile_shaders(${PROJECT_NAME}
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders"
        "${CMAKE_BINARY_DIR}/shaders"
)