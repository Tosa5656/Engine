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
    void Render(VkCommandBuffer commandBuffer);
    void Shutdown();

private:
    VkDevice m_device;
    VkInstance m_instance;
    VkPhysicalDevice m_physicalDevice;
    uint32_t m_graphicsQueueFamilyIndex;
    VkQueue m_graphicsQueue;
    GLFWwindow* m_window;
    VkRenderPass m_renderPass;
};

#endif