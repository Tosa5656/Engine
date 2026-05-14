cmake_minimum_required(VERSION 4.0)

set(PROJECT_NAME Singularity)
project(${PROJECT_NAME}
        VERSION 0.4.0
        DESCRIPTION "Singularity Game Engine"
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

set(PROJECT_SOURCES src/main.cpp
                    src/window/window.cpp
                    src/renderer/renderer.cpp
                    src/gui-debug/imgui/imgui.cpp
                    src/vma/vma_impl.cpp
                    src/renderer/vulkan/instance.cpp
                    src/renderer/vulkan/device.cpp
                    src/renderer/vulkan/swapchain.cpp
                    src/renderer/vulkan/commandbuffer.cpp
                    src/renderer/vulkan/pipeline.cpp
                    src/renderer/vulkan/descriptors.cpp
                    src/renderer/vulkan/resources.cpp
                    src/renderer/vulkan/surface.cpp
                    src/renderer/vulkan/mesh.cpp
                    src/utils/vars/transform.cpp
                    src/renderer/vulkan/object.cpp
                    src/renderer/vulkan/scene.cpp
                    src/renderer/vulkan/camera.cpp
                    src/renderer/vulkan/material.cpp
                    src/renderer/vulkan/texture.cpp
                    src/renderer/vulkan/texturearray.cpp
                    src/renderer/vulkan/stb_image_impl.cpp
                    src/utils/input/input.cpp
                    src/renderer/vulkan/light/light.cpp
                    src/renderer/vulkan/light/directional.cpp
                    src/renderer/vulkan/light/point.cpp
                    src/renderer/vulkan/light/spot.cpp
                    src/renderer/vulkan/light/debug_mesh.cpp)
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
        GIT_TAG        1.0.1
)
FetchContent_MakeAvailable(glm)

target_link_libraries(${PROJECT_NAME} PRIVATE glm::glm)

FetchContent_Declare(
        VulkanMemoryAllocator
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
        GIT_TAG        master
)

FetchContent_MakeAvailable(VulkanMemoryAllocator)

FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_TAG master
)
FetchContent_MakeAvailable(stb)

include_directories(${stb_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE GPUOpen::VulkanMemoryAllocator)

FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG        master
)

FetchContent_MakeAvailable(imgui)

set(IMGUI_SOURCES
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_demo.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
)

add_library(imgui STATIC ${IMGUI_SOURCES})

target_include_directories(imgui PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(imgui PUBLIC Vulkan::Vulkan)
target_link_libraries(imgui PUBLIC glfw)

target_link_libraries(${PROJECT_NAME} PUBLIC imgui)

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

function(copy_resources RESOURCE_DIR OUTPUT_DIR)
    file(MAKE_DIRECTORY ${OUTPUT_DIR})

    file(GLOB_RECURSE RESOURCES "${RESOURCE_DIR}/*")

    foreach(RESOURCE ${RESOURCES})
        if(NOT IS_DIRECTORY ${RESOURCE})
            file(COPY ${RESOURCE} DESTINATION ${OUTPUT_DIR})
        endif()
    endforeach()
endfunction()

copy_resources("${CMAKE_CURRENT_SOURCE_DIR}/resources/models"
        "${CMAKE_BINARY_DIR}/models"
)

copy_resources("${CMAKE_CURRENT_SOURCE_DIR}/resources/textures"
        "${CMAKE_BINARY_DIR}/textures"
)