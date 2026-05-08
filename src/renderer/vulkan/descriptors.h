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
#include <renderer/vulkan/texture.h>
#include <renderer/vulkan/texturearray.h>

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
    VkDescriptorSetLayout GetTextureSetLayout() const { return m_textureSetLayout; }
    VkDescriptorSetLayout GetNormalMapSetLayout() const { return m_normalMapSetLayout; }
    VkDescriptorSetLayout GetHeightMapSetLayout() const { return m_heightMapSetLayout; }
    VkDescriptorSet GetNullTextureDescriptorSet() const { return m_nullTextureDescriptorSet; }
    VkDescriptorSet GetNullNormalMapDescriptorSet() const { return m_nullNormalMapDescriptorSet; }
    VkDescriptorSet GetNullHeightMapDescriptorSet() const { return m_nullHeightMapDescriptorSet; }

    VkDescriptorSet CreateTextureDescriptorSet(Texture* texture);
    VkDescriptorSet CreateTextureDescriptorSet(TextureArray* textureArray);
    VkDescriptorSet CreateNormalMapDescriptorSet(Texture* texture);
    VkDescriptorSet CreateHeightMapDescriptorSet(Texture* texture);

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
    VkDescriptorSetLayout m_textureSetLayout;
    VkDescriptorSetLayout m_normalMapSetLayout;
    VkDescriptorSetLayout m_heightMapSetLayout;
    VkDescriptorSet m_nullTextureDescriptorSet;
    VkDescriptorSet m_nullNormalMapDescriptorSet;
    VkDescriptorSet m_nullHeightMapDescriptorSet;
    VkSampler m_dummySampler;
    VkImageView m_dummyImageView;
    VkDeviceMemory m_dummyImageMemory;
    VkDescriptorSet m_computeDescriptorSet;
};