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

    VkDescriptorPool GetDescriptorPool();
    VkDescriptorSetLayout GetDescriptorSetLayout(uint32_t index);
    std::vector<VkDescriptorSet> GetDescriptorSets();
    std::vector<VkDescriptorSet> GetPerObjectDescriptorSets();
    VkDescriptorSetLayout GetTextureSetLayout() const { return m_textureSetLayout; }
    VkDescriptorSetLayout GetLightSetLayout() const { return m_lightSetLayout; }
    VkDescriptorSet GetLightDescriptorSet() const { return m_lightDescriptorSet; }
    VkDescriptorSetLayout GetNormalMapSetLayout() const { return m_normalMapSetLayout; }
    VkDescriptorSetLayout GetHeightMapSetLayout() const { return m_heightMapSetLayout; }
    VkDescriptorSet GetNullTextureDescriptorSet() const { return m_nullTextureDescriptorSet; }
    VkDescriptorSet GetNullNormalMapDescriptorSet() const { return m_nullNormalMapDescriptorSet; }
    VkDescriptorSet GetNullHeightMapDescriptorSet() const { return m_nullHeightMapDescriptorSet; }

    VkDescriptorSetLayout GetGBufferSetLayout() const { return m_gbufferSetLayout; }
    VkDescriptorSetLayout GetCompositeSetLayout() const { return m_compositeSetLayout; }
    VkDescriptorSet GetGBufferDescriptorSet() const { return m_gbufferDescriptorSet; }
    VkDescriptorSet GetCompositeDescriptorSet() const { return m_compositeDescriptorSet; }
    VkDescriptorSet GetEmissiveAccumDescriptorSet() const { return m_emissiveAccumDescriptorSet; }

    void CreateGBufferDescriptorSet();
    void CreateCompositeDescriptorSet();
    void UpdateGBufferDescriptorSet();
    void UpdateCompositeDescriptorSet();

    VkDescriptorSet CreateTextureDescriptorSet(Texture* texture);
    VkDescriptorSet CreateTextureDescriptorSet(TextureArray* textureArray);
    VkDescriptorSet CreateNormalMapDescriptorSet(Texture* texture);
    VkDescriptorSet CreateHeightMapDescriptorSet(Texture* texture);

    void CreateClusterSetLayout();
    void CreateClusterDescriptorSet();
    VkDescriptorSetLayout GetClusterSetLayout() const { return m_clusterSetLayout; }
    VkDescriptorSet GetClusterDescriptorSet() const { return m_clusterDescriptorSet; }

    VkDescriptorSet GetHdrDescriptorSet() const { return m_hdrDescriptorSet; }
    void CreateHdrDescriptorSet();
    void UpdateHdrDescriptorSet();

private:
    Device* m_device;
    SwapChain* m_swapChain;
    ResourceManager* m_resourceManager;

    VkDescriptorPool m_descriptorPool;
    std::vector<VkDescriptorSet> m_perFrameDescriptorSets;
    std::vector<VkDescriptorSet> m_perObjectDescriptorSets;
    VkDescriptorSetLayout m_perFrameSetLayout;
    VkDescriptorSetLayout m_perObjectSetLayout;
    VkDescriptorSetLayout m_textureSetLayout;
    VkDescriptorSetLayout m_lightSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_lightDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_normalMapSetLayout;
    VkDescriptorSetLayout m_heightMapSetLayout;
    VkDescriptorSet m_nullTextureDescriptorSet;
    VkDescriptorSet m_nullNormalMapDescriptorSet;
    VkDescriptorSet m_nullHeightMapDescriptorSet;
    VkSampler m_dummySampler;
    VkImageView m_dummyImageView;
    VkDeviceMemory m_dummyImageMemory;

    VkDescriptorSetLayout m_gbufferSetLayout = VK_NULL_HANDLE;
    VkDescriptorSetLayout m_compositeSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_gbufferDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet m_compositeDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet m_emissiveAccumDescriptorSet = VK_NULL_HANDLE;

    VkDescriptorSetLayout m_clusterSetLayout = VK_NULL_HANDLE;
    VkDescriptorSet m_clusterDescriptorSet = VK_NULL_HANDLE;

    VkDescriptorSet m_hdrDescriptorSet = VK_NULL_HANDLE;
};