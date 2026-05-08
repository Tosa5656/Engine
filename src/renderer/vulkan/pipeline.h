#pragma once

#include <iostream>
#include <vector>
#include <fstream>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>
#include <renderer/vulkan/resources.h>

class PipelineManager
{
public:
    PipelineManager();
    ~PipelineManager();

    void Create(Device* device, SwapChain* swapChain, VkDescriptorSetLayout perFrameSetLayout, VkDescriptorSetLayout perObjectSetLayout, VkDescriptorSetLayout textureSetLayout, VkDescriptorSetLayout normalMapSetLayout, VkDescriptorSetLayout heightMapSetLayout);
    void Shutdown(Device* device);
    VkShaderModule CreateShaderModule(const std::vector<char> &code, Device* device);

    VkPipeline GetGraphicsPipeline();
    VkPipelineLayout GetPipelineLayout();
private:
    std::vector<char> ReadFile(const std::string& filename);

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkPipelineRenderingCreateInfo m_pipelineRenderingCreateInfo{};
};