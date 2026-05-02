#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>
#include <renderer/vulkan/resources.h>

class DescriptorsManager
{
public:
    DescriptorsManager();
    ~DescriptorsManager();

    void Init(Device* device, SwapChain* swapChain, ResourceManager* resourceManager, uint32_t maxObjects);
    void Cleanup();

    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void CreateDescriptorSetLayout();
    void CreateComputeDescriptorSetLayout();
    void CreateComputeDescriptorSet();

    VkDescriptorPool GetDescriptorPool();
    VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index);
    std::vector<VkDescriptorSet> GetDescriptorSets();
    std::vector<VkDescriptorSet> GetPerObjectDescriptorSets();
    VkDescriptorSetLayout GetComputeDescriptorSetLayout();
    VkDescriptorSet GetComputeDescriptorSet();

private:
    Device* m_device;
    SwapChain* m_swapChain;
    ResourceManager* m_resourceManager;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_perFrameDescriptorSets;
    std::vector<VkDescriptorSet> m_perObjectDescriptorSets;
    VkDescriptorSetLayout m_perFrameSetLayout;
    VkDescriptorSetLayout m_perObjectSetLayout;
    VkDescriptorSetLayout m_computeSetLayout;
    VkDescriptorSet m_computeDescriptorSet;
};