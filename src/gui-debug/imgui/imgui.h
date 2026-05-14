#pragma once
#ifndef SINGULARITY_IMGUI_H
#define SINGULARITY_IMGUI_H

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>

class GUI
{
public:
    void Init(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, GLFWwindow* window, VkRenderPass renderPass);
    void NewFrame();
    void DebugConsole();
    void DebugInfo(int fps);
    void Render(VkCommandBuffer commandBuffer);
    void Shutdown();
    void SetShowCursor(bool show);
    void UpdateStats(float deltaTime, uint64_t gpuMemory, uint64_t cpuMemory);

private:
    VkDevice m_device;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    uint32_t m_graphicsQueueFamilyIndex;
    VkQueue m_graphicsQueue;
    GLFWwindow* m_window;
    VkRenderPass m_renderPass;

    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    float m_deltaTime = 0.0f;
    uint64_t m_gpuMemory = 0;
    uint64_t m_cpuMemory = 0;
    int m_drawCalls = 0;
};

#endif