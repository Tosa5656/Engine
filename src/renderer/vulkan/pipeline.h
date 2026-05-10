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

    void Create(Device* device, SwapChain* swapChain, VkDescriptorSetLayout perFrameSetLayout, VkDescriptorSetLayout perObjectSetLayout, VkDescriptorSetLayout textureSetLayout, VkDescriptorSetLayout normalMapSetLayout, VkDescriptorSetLayout heightMapSetLayout, VkDescriptorSetLayout lightSetLayout);
    void CreateLinePipeline(Device* device, SwapChain* swapChain);
    void CreateGBufferPipeline(Device* device, SwapChain* swapChain, VkDescriptorSetLayout perFrameSetLayout, VkDescriptorSetLayout perObjectSetLayout, VkDescriptorSetLayout textureSetLayout, VkDescriptorSetLayout normalMapSetLayout, VkDescriptorSetLayout heightMapSetLayout);
    void CreateLightingPipeline(Device* device, SwapChain* swapChain, VkDescriptorSetLayout perFrameSetLayout, VkDescriptorSetLayout gbufferSetLayout, VkDescriptorSetLayout lightSetLayout);
    void CreateCompositePipeline(Device* device, SwapChain* swapChain, VkDescriptorSetLayout compositeSetLayout);
    void Shutdown(Device* device);
    VkShaderModule CreateShaderModule(const std::vector<char> &code, Device* device);

    VkPipeline GetGraphicsPipeline();
    VkPipeline GetLinePipeline() const { return m_lineGraphicsPipeline; }
    VkPipelineLayout GetPipelineLayout();
    VkPipeline GetGBufferPipeline() const { return m_gbufferPipeline; }
    VkPipelineLayout GetGBufferPipelineLayout() const { return m_gbufferPipelineLayout; }
    VkPipeline GetLightingPipeline() const { return m_lightingPipeline; }
    VkPipelineLayout GetLightingPipelineLayout() const { return m_lightingPipelineLayout; }
    VkPipeline GetCompositePipeline() const { return m_compositePipeline; }
    VkPipelineLayout GetCompositePipelineLayout() const { return m_compositePipelineLayout; }
private:
    std::vector<char> ReadFile(const std::string& filename);

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_graphicsPipeline;
    VkPipeline m_lineGraphicsPipeline = VK_NULL_HANDLE;
    VkPipelineRenderingCreateInfo m_pipelineRenderingCreateInfo{};

    VkPipelineLayout m_gbufferPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_gbufferPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_lightingPipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_lightingPipeline = VK_NULL_HANDLE;
    VkPipelineLayout m_compositePipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_compositePipeline = VK_NULL_HANDLE;
};