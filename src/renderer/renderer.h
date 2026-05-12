#pragma once

#include <iostream>
#include <vector>
#include <ranges>
#include <optional>
#include <set>
#include <cstdint>
#include <fstream>
#include <array>
#include <chrono>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/instance.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/surface.h>
#include <renderer/vulkan/swapchain.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/pipeline.h>
#include <renderer/vulkan/descriptors.h>
#include <renderer/vulkan/mesh.h>
#include <renderer/vulkan/resources.h>
#include <renderer/vulkan/object.h>
#include <renderer/vulkan/material.h>
#include <renderer/vulkan/camera.h>
#include <renderer/vulkan/scene.h>
#include <renderer/vulkan/texture.h>
#include <renderer/vulkan/texturearray.h>

#include <utils/input/input.h>
#include <gui-debug/imgui/imgui.h>

static bool is_glfw_initialized = false;

static void InitGLFW()
{
    if (is_glfw_initialized)
        return;
    else
    {
        if (glfwInit())
            is_glfw_initialized = true;
    }
}

static void DestroyGLFW()
{
    if (is_glfw_initialized)
    {
        glfwTerminate();
        is_glfw_initialized = false;
    }
    else
        return;
}

struct FrameData
{
    VkSemaphore imageAvailableSemaphore = VK_NULL_HANDLE;
    VkSemaphore renderFinishedSemaphore = VK_NULL_HANDLE;
    VkFence     inFlightFence           = VK_NULL_HANDLE;
};

class Renderer
{
public:
    Renderer();
    ~Renderer();

    void Init(GLFWwindow* window, Input* input);
    void Render();
    void Destroy();

    void SetFramebufferResized(bool resized);
    void SetShowCursor(bool show);

    VkDevice GetDevice();
    float GetDeltaTime();
private:
    void CreateSyncObjects();
    void CreatePerImageSemaphores();

    GLFWwindow* m_window;

    Instance m_instance;
    Device m_device;
    Surface m_surface;
    SwapChain m_swapChain;
    CommandBufferManager m_commandBufferManager;
    PipelineManager m_pipelineManager;
    DescriptorsManager m_descriptorManager;
    ResourceManager m_resourceManager;
    Mesh m_mesh;

    Scene m_scene;
    Material m_material;
    Material m_material2;
    Material m_material3;
    Material m_material4;
    Material m_material5;
    TextureArray m_textureAtlas;
    Texture m_singleTexture;
    Texture m_normalMap;
    Texture m_heightMap;
    Input* m_input;

    GUI m_gui;

    bool  m_autoExposure = false;
    float m_manualExposure = 1.0f;
    float m_targetLuminance = 0.18f;
    float m_minExposure = 0.05f;
    float m_maxExposure = 3.0f;
    float m_adaptSpeed = 1.0f;
    float m_smoothedExposure = 1.0f;
    bool  m_luminanceValid = false;

#ifndef NDEBUG
    Mesh m_debugSphere;
    Mesh m_debugCone;
    Mesh m_debugArrow;
    bool m_debugLightsEnabled = true;
#endif
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
    bool m_framebufferResized = false;
    float m_deltaTime = 0.0f;
    float m_fps = 0.0f;
    float m_fpsAccumulator = 0.0f;
    int m_fpsFrameCount = 0;
};