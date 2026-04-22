#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>

class DescriptorsManager
{
public:
    DescriptorsManager();
    ~DescriptorsManager();

    void CreateDescriptorPool(Device* device, SwapChain* swapChain);
    void CreateDescriptorSets(Device* device, SwapChain* swapChain);
    void CreateDescriptorSetLayout(Device* device);
    void Cleanup(Device* device);

    VkDescriptorPool GetDescriptorPool();
    std::vector<VkDescriptorSet> GetDescriptorSets();
    VkDescriptorSetLayout GetDescriptorSetLayout();
    std::vector<VkBuffer>& GetUniformBuffers(); // TODO: transfer to resource manager class
private:
    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    VkDescriptorSetLayout m_descriptorSetLayout;
    std::vector<VkBuffer> m_uniformBuffers; // TODO: Transfer to new class
};