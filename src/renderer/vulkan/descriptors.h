#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>

class ResourceManager;

class DescriptorsManager
{
public:
    DescriptorsManager();
    ~DescriptorsManager();

    void Init(Device* device, SwapChain* swapChain, ResourceManager* resourceManager);
    void Cleanup();

    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateDescriptorSetLayout();

    VkDescriptorPool GetDescriptorPool();
    std::vector<VkDescriptorSet> GetDescriptorSets();
    VkDescriptorSetLayout GetDescriptorSetLayout();
private:
    Device* m_device;
    SwapChain* m_swapChain;
    ResourceManager* m_resourceManager;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_descriptorSets;
    VkDescriptorSetLayout m_descriptorSetLayout;
};
