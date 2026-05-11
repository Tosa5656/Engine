#include "descriptors.h"
#include "resources.h"
#include <array>

DescriptorsManager::DescriptorsManager() : m_dummySampler(VK_NULL_HANDLE), m_dummyImageView(VK_NULL_HANDLE), m_dummyImageMemory(VK_NULL_HANDLE), m_normalMapSetLayout(VK_NULL_HANDLE), m_nullNormalMapDescriptorSet(VK_NULL_HANDLE), m_heightMapSetLayout(VK_NULL_HANDLE), m_nullHeightMapDescriptorSet(VK_NULL_HANDLE) {}
DescriptorsManager::~DescriptorsManager() {}

void DescriptorsManager::Init(Device* device, SwapChain* swapChain, ResourceManager* resourceManager, uint32_t maxObjects)
{
    m_device = device;
    m_swapChain = swapChain;
    m_resourceManager = resourceManager;
}

void DescriptorsManager::Cleanup()
{
    if (!m_device)
        return;

    VkDevice device = m_device->GetDevice();
    if (device == VK_NULL_HANDLE)
        return;

    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    if (m_perFrameSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_perFrameSetLayout, nullptr);
        m_perFrameSetLayout = VK_NULL_HANDLE;
    }

    if (m_perObjectSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_perObjectSetLayout, nullptr);
        m_perObjectSetLayout = VK_NULL_HANDLE;
    }

    if (m_lightSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_lightSetLayout, nullptr);
        m_lightSetLayout = VK_NULL_HANDLE;
    }

    if (m_textureSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_textureSetLayout, nullptr);
        m_textureSetLayout = VK_NULL_HANDLE;
    }

    if (m_normalMapSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_normalMapSetLayout, nullptr);
        m_normalMapSetLayout = VK_NULL_HANDLE;
    }

    if (m_heightMapSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_heightMapSetLayout, nullptr);
        m_heightMapSetLayout = VK_NULL_HANDLE;
    }

    if (m_dummySampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, m_dummySampler, nullptr);
        m_dummySampler = VK_NULL_HANDLE;
    }

    if (m_dummyImageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, m_dummyImageView, nullptr);
        m_dummyImageView = VK_NULL_HANDLE;
    }

    if (m_dummyImageMemory != VK_NULL_HANDLE)
    {
        vkFreeMemory(device, m_dummyImageMemory, nullptr);
    }
    m_dummyImageMemory = VK_NULL_HANDLE;

    if (m_gbufferSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_gbufferSetLayout, nullptr);
        m_gbufferSetLayout = VK_NULL_HANDLE;
    }

    if (m_compositeSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_compositeSetLayout, nullptr);
        m_compositeSetLayout = VK_NULL_HANDLE;
    }

    if (m_clusterSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_clusterSetLayout, nullptr);
        m_clusterSetLayout = VK_NULL_HANDLE;
    }

    if (m_luminanceSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_luminanceSetLayout, nullptr);
        m_luminanceSetLayout = VK_NULL_HANDLE;
    }
}

void DescriptorsManager::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(13);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = 256;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = 4;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = 256;
    poolSizes[4].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[4].descriptorCount = 256;
    poolSizes[5].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[5].descriptorCount = 256;
    poolSizes[6].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[6].descriptorCount = 1;
    poolSizes[7].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[7].descriptorCount = 2;
    poolSizes[8].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[8].descriptorCount = 5;
    poolSizes[9].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[9].descriptorCount = 2;
    poolSizes[10].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[10].descriptorCount = 1;

    poolSizes[11].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[11].descriptorCount = 2;

    poolSizes[12].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[12].descriptorCount = 2;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size()) + 2 + 256 + 256 + 256 + 1 + 1 + 2 + 1 + 1;

    if (vkCreateDescriptorPool(m_device->GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorsManager::CreateDescriptorSets()
{
    uint32_t imageCount = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size());

    m_perFrameDescriptorSets.resize(imageCount);
    m_perObjectDescriptorSets.resize(1);

    std::vector<VkDescriptorSetLayout> perFrameLayouts(imageCount, m_perFrameSetLayout);
    std::vector<VkDescriptorSetLayout> perObjectLayouts(1, m_perObjectSetLayout);

    std::vector<VkDescriptorSet> allSets(imageCount + 1);

    std::vector<VkDescriptorSetLayout> allLayouts;
    allLayouts.insert(allLayouts.end(), perFrameLayouts.begin(), perFrameLayouts.end());
    allLayouts.insert(allLayouts.end(), perObjectLayouts.begin(), perObjectLayouts.end());

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = imageCount + 1;
    allocInfo.pSetLayouts = allLayouts.data();

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, allSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        m_perFrameDescriptorSets[i] = allSets[i];
    }
    m_perObjectDescriptorSets[0] = allSets[imageCount];

    for (size_t i = 0; i < imageCount; i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_resourceManager->GetPerFrameBuffers()[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(PerFrameUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_perFrameDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_resourceManager->GetObjectBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = m_resourceManager->GetObjectUBOStride();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_perObjectDescriptorSets[0];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }

    VkDescriptorSetAllocateInfo lightAllocInfo{};
    lightAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    lightAllocInfo.descriptorPool = m_descriptorPool;
    lightAllocInfo.descriptorSetCount = 1;
    lightAllocInfo.pSetLayouts = &m_lightSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &lightAllocInfo, &m_lightDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate light descriptor set!");

    VkDescriptorBufferInfo lightBufferInfo{};
    lightBufferInfo.buffer = m_resourceManager->GetLightSSBO();
    lightBufferInfo.offset = 0;
    lightBufferInfo.range = m_resourceManager->GetLightBufferSize();

    VkWriteDescriptorSet lightDescriptorWrite{};
    lightDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    lightDescriptorWrite.dstSet = m_lightDescriptorSet;
    lightDescriptorWrite.dstBinding = 0;
    lightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightDescriptorWrite.descriptorCount = 1;
    lightDescriptorWrite.pBufferInfo = &lightBufferInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &lightDescriptorWrite, 0, nullptr);

    VkDescriptorSetAllocateInfo nullAllocInfo{};
    nullAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    nullAllocInfo.descriptorPool = m_descriptorPool;
    nullAllocInfo.descriptorSetCount = 1;
    nullAllocInfo.pSetLayouts = &m_textureSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &nullAllocInfo, &m_nullTextureDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate null texture descriptor set!");

    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;

    if (vkCreateSampler(m_device->GetDevice(), &samplerInfo, nullptr, &m_dummySampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create dummy sampler!");

    VkImage dummyImage;
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = 1;
    imageInfo.extent.height = 1;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(m_device->GetDevice(), &imageInfo, nullptr, &dummyImage) != VK_SUCCESS)
        throw std::runtime_error("failed to create dummy image!");

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device->GetDevice(), dummyImage, &memRequirements);

    VkMemoryAllocateInfo memoryAllocInfo{};
    memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocInfo.allocationSize = memRequirements.size;
    memoryAllocInfo.memoryTypeIndex = m_device->FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    VkDeviceMemory dummyMemory;
    if (vkAllocateMemory(m_device->GetDevice(), &memoryAllocInfo, nullptr, &dummyMemory) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate dummy image memory!");

    if (vkBindImageMemory(m_device->GetDevice(), dummyImage, dummyMemory, 0) != VK_SUCCESS)
        throw std::runtime_error("failed to bind dummy image memory!");

    m_dummyImageMemory = dummyMemory;

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = dummyImage;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device->GetDevice(), &viewInfo, nullptr, &m_dummyImageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create dummy image view!");

    vkDestroyImage(m_device->GetDevice(), dummyImage, nullptr);

    VkDescriptorImageInfo nullImageInfo{};
    nullImageInfo.sampler = m_dummySampler;
    nullImageInfo.imageView = m_dummyImageView;
    nullImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet nullDescriptorWrite{};
    nullDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    nullDescriptorWrite.dstSet = m_nullTextureDescriptorSet;
    nullDescriptorWrite.dstBinding = 0;
    nullDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    nullDescriptorWrite.descriptorCount = 1;
    nullDescriptorWrite.pImageInfo = &nullImageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &nullDescriptorWrite, 0, nullptr);

    VkDescriptorSetAllocateInfo nullNormalAllocInfo{};
    nullNormalAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    nullNormalAllocInfo.descriptorPool = m_descriptorPool;
    nullNormalAllocInfo.descriptorSetCount = 1;
    nullNormalAllocInfo.pSetLayouts = &m_normalMapSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &nullNormalAllocInfo, &m_nullNormalMapDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate null normal map descriptor set!");

    VkDescriptorImageInfo nullNormalImageInfo{};
    nullNormalImageInfo.sampler = m_dummySampler;
    nullNormalImageInfo.imageView = m_dummyImageView;
    nullNormalImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet nullNormalDescriptorWrite{};
    nullNormalDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    nullNormalDescriptorWrite.dstSet = m_nullNormalMapDescriptorSet;
    nullNormalDescriptorWrite.dstBinding = 0;
    nullNormalDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    nullNormalDescriptorWrite.descriptorCount = 1;
    nullNormalDescriptorWrite.pImageInfo = &nullNormalImageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &nullNormalDescriptorWrite, 0, nullptr);

    VkDescriptorSetAllocateInfo nullHeightAllocInfo{};
    nullHeightAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    nullHeightAllocInfo.descriptorPool = m_descriptorPool;
    nullHeightAllocInfo.descriptorSetCount = 1;
    nullHeightAllocInfo.pSetLayouts = &m_heightMapSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &nullHeightAllocInfo, &m_nullHeightMapDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate null height map descriptor set!");

    VkDescriptorImageInfo nullHeightImageInfo{};
    nullHeightImageInfo.sampler = m_dummySampler;
    nullHeightImageInfo.imageView = m_dummyImageView;
    nullHeightImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet nullHeightDescriptorWrite{};
    nullHeightDescriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    nullHeightDescriptorWrite.dstSet = m_nullHeightMapDescriptorSet;
    nullHeightDescriptorWrite.dstBinding = 0;
    nullHeightDescriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    nullHeightDescriptorWrite.descriptorCount = 1;
    nullHeightDescriptorWrite.pImageInfo = &nullHeightImageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &nullHeightDescriptorWrite, 0, nullptr);
}

void DescriptorsManager::CreateDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding perFrameBinding{};
    perFrameBinding.binding = 0;
    perFrameBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    perFrameBinding.descriptorCount = 1;
    perFrameBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo perFrameLayoutInfo{};
    perFrameLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    perFrameLayoutInfo.bindingCount = 1;
    perFrameLayoutInfo.pBindings = &perFrameBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &perFrameLayoutInfo, nullptr, &m_perFrameSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create per-frame descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding perObjectBinding{};
    perObjectBinding.binding = 0;
    perObjectBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    perObjectBinding.descriptorCount = 1;
    perObjectBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo perObjectLayoutInfo{};
    perObjectLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    perObjectLayoutInfo.bindingCount = 1;
    perObjectLayoutInfo.pBindings = &perObjectBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &perObjectLayoutInfo, nullptr, &m_perObjectSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create per-object descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding textureBinding{};
    textureBinding.binding = 0;
    textureBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    textureBinding.descriptorCount = 1;
    textureBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo textureLayoutInfo{};
    textureLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    textureLayoutInfo.bindingCount = 1;
    textureLayoutInfo.pBindings = &textureBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &textureLayoutInfo, nullptr, &m_textureSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create texture descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding normalMapBinding{};
    normalMapBinding.binding = 0;
    normalMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    normalMapBinding.descriptorCount = 1;
    normalMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo normalMapLayoutInfo{};
    normalMapLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    normalMapLayoutInfo.bindingCount = 1;
    normalMapLayoutInfo.pBindings = &normalMapBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &normalMapLayoutInfo, nullptr, &m_normalMapSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create normal map descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding heightMapBinding{};
    heightMapBinding.binding = 0;
    heightMapBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    heightMapBinding.descriptorCount = 1;
    heightMapBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo heightMapLayoutInfo{};
    heightMapLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    heightMapLayoutInfo.bindingCount = 1;
    heightMapLayoutInfo.pBindings = &heightMapBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &heightMapLayoutInfo, nullptr, &m_heightMapSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create height map descriptor set layout!");
    }

    VkDescriptorSetLayoutBinding lightBinding{};
    lightBinding.binding = 0;
    lightBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    lightBinding.descriptorCount = 1;
    lightBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo lightLayoutInfo{};
    lightLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    lightLayoutInfo.bindingCount = 1;
    lightLayoutInfo.pBindings = &lightBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &lightLayoutInfo, nullptr, &m_lightSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create light descriptor set layout!");
    }

    std::array<VkDescriptorSetLayoutBinding, 5> gbufferBindings{};
    for (uint32_t i = 0; i < 5; i++)
    {
        gbufferBindings[i].binding = i;
        gbufferBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        gbufferBindings[i].descriptorCount = 1;
        gbufferBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }

    VkDescriptorSetLayoutCreateInfo gbufferLayoutInfo{};
    gbufferLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    gbufferLayoutInfo.bindingCount = 5;
    gbufferLayoutInfo.pBindings = gbufferBindings.data();

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &gbufferLayoutInfo, nullptr, &m_gbufferSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create g-buffer descriptor set layout!");

    VkDescriptorSetLayoutBinding compositeBindings[2]{};
    compositeBindings[0].binding = 0;
    compositeBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    compositeBindings[0].descriptorCount = 1;
    compositeBindings[0].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    compositeBindings[1].binding = 0;
    compositeBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    compositeBindings[1].descriptorCount = 1;
    compositeBindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo compositeLayoutInfo{};
    compositeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    compositeLayoutInfo.bindingCount = 1;
    compositeLayoutInfo.pBindings = compositeBindings;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &compositeLayoutInfo, nullptr, &m_compositeSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create composite descriptor set layout!");
}

void DescriptorsManager::CreateClusterSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 5> bindings{};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[2].binding = 2;
    bindings[2].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[2].descriptorCount = 1;
    bindings[2].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[3].binding = 3;
    bindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[3].descriptorCount = 1;
    bindings[3].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    bindings[4].binding = 4;
    bindings[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[4].descriptorCount = 1;
    bindings[4].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &layoutInfo, nullptr, &m_clusterSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create cluster descriptor set layout!");
}

void DescriptorsManager::CreateClusterDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_clusterSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_clusterDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate cluster descriptor set!");

    std::array<VkWriteDescriptorSet, 5> writes{};
    std::array<VkDescriptorBufferInfo, 4> bufferInfos{};
    VkDescriptorImageInfo imageInfo{};

    bufferInfos[0].buffer = m_resourceManager->GetLightSSBO();
    bufferInfos[0].offset = 0;
    bufferInfos[0].range = m_resourceManager->GetLightBufferSize();

    bufferInfos[1].buffer = m_resourceManager->GetClusterCountSSBO();
    bufferInfos[1].offset = 0;
    bufferInfos[1].range = VK_WHOLE_SIZE;

    bufferInfos[2].buffer = m_resourceManager->GetClusterIndexSSBO();
    bufferInfos[2].offset = 0;
    bufferInfos[2].range = VK_WHOLE_SIZE;

    bufferInfos[3].buffer = m_resourceManager->GetClusterGridInfoUBO();
    bufferInfos[3].offset = 0;
    bufferInfos[3].range = sizeof(ClusterGridBuffer);

    imageInfo.sampler = m_dummySampler;
    imageInfo.imageView = m_swapChain->GetDepthImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    for (uint32_t i = 0; i < 4; i++)
    {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = m_clusterDescriptorSet;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = i < 3 ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[i].pBufferInfo = &bufferInfos[i];
    }

    writes[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[4].dstSet = m_clusterDescriptorSet;
    writes[4].dstBinding = 4;
    writes[4].descriptorCount = 1;
    writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[4].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DescriptorsManager::UpdateClusterDescriptorSet()
{
    if (m_clusterDescriptorSet == VK_NULL_HANDLE) return;

    std::array<VkWriteDescriptorSet, 5> writes{};
    std::array<VkDescriptorBufferInfo, 4> bufferInfos{};
    VkDescriptorImageInfo imageInfo{};

    bufferInfos[0].buffer = m_resourceManager->GetLightSSBO();
    bufferInfos[0].offset = 0;
    bufferInfos[0].range = m_resourceManager->GetLightBufferSize();

    bufferInfos[1].buffer = m_resourceManager->GetClusterCountSSBO();
    bufferInfos[1].offset = 0;
    bufferInfos[1].range = VK_WHOLE_SIZE;

    bufferInfos[2].buffer = m_resourceManager->GetClusterIndexSSBO();
    bufferInfos[2].offset = 0;
    bufferInfos[2].range = VK_WHOLE_SIZE;

    bufferInfos[3].buffer = m_resourceManager->GetClusterGridInfoUBO();
    bufferInfos[3].offset = 0;
    bufferInfos[3].range = sizeof(ClusterGridBuffer);

    imageInfo.sampler = m_dummySampler;
    imageInfo.imageView = m_swapChain->GetDepthImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;

    for (uint32_t i = 0; i < 4; i++)
    {
        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].dstSet = m_clusterDescriptorSet;
        writes[i].dstBinding = i;
        writes[i].descriptorCount = 1;
        writes[i].descriptorType = i < 3 ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        writes[i].pBufferInfo = &bufferInfos[i];
    }

    writes[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[4].dstSet = m_clusterDescriptorSet;
    writes[4].dstBinding = 4;
    writes[4].descriptorCount = 1;
    writes[4].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[4].pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

VkDescriptorPool DescriptorsManager::GetDescriptorPool()
{
    return m_descriptorPool;
}

VkDescriptorSetLayout DescriptorsManager::GetDescriptorSetLayout(uint32_t index)
{
    return index == 0 ? m_perFrameSetLayout : m_perObjectSetLayout;
}

std::vector<VkDescriptorSet> DescriptorsManager::GetDescriptorSets()
{
    return m_perFrameDescriptorSets;
}

std::vector<VkDescriptorSet> DescriptorsManager::GetPerObjectDescriptorSets()
{
    return m_perObjectDescriptorSets;
}

VkDescriptorSet DescriptorsManager::CreateTextureDescriptorSet(Texture* texture)
{
    VkDescriptorSet descriptorSet;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_textureSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate texture descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->GetImageView();
    imageInfo.sampler = texture->GetSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);

    return descriptorSet;
}

VkDescriptorSet DescriptorsManager::CreateHeightMapDescriptorSet(Texture* texture)
{
    VkDescriptorSet descriptorSet;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_heightMapSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate height map descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->GetImageView();
    imageInfo.sampler = texture->GetSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);

    return descriptorSet;
}

VkDescriptorSet DescriptorsManager::CreateNormalMapDescriptorSet(Texture* texture)
{
    VkDescriptorSet descriptorSet;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_normalMapSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate normal map descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture->GetImageView();
    imageInfo.sampler = texture->GetSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);

    return descriptorSet;
}

VkDescriptorSet DescriptorsManager::CreateTextureDescriptorSet(TextureArray* textureArray)
{
    VkDescriptorSet descriptorSet;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_textureSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &descriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate texture descriptor set!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = textureArray->GetImageView();
    imageInfo.sampler = textureArray->GetSampler();

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = descriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);

    return descriptorSet;
}

void DescriptorsManager::CreateGBufferDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_gbufferSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_gbufferDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate g-buffer descriptor set!");

    VkImageView gBufferViews[5] = {
        m_swapChain->GetPositionImageView(),
        m_swapChain->GetNormalImageView(),
        m_swapChain->GetAlbedoImageView(),
        m_swapChain->GetMaterialImageView(),
        m_swapChain->GetDepthImageView()
    };

    VkDescriptorImageInfo imageInfos[5]{};
    VkWriteDescriptorSet writes[5]{};
    for (uint32_t i = 0; i < 5; i++)
    {
        imageInfos[i].sampler = m_dummySampler;
        imageInfos[i].imageView = gBufferViews[i];
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].pNext = nullptr;
        writes[i].dstSet = m_gbufferDescriptorSet;
        writes[i].dstBinding = i;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].descriptorCount = 1;
        writes[i].pImageInfo = &imageInfos[i];
    }

    vkUpdateDescriptorSets(m_device->GetDevice(), 5, writes, 0, nullptr);
}

void DescriptorsManager::CreateCompositeDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_compositeSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_compositeDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate composite descriptor set (lighting result)!");

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_dummySampler;
    imageInfo.imageView = m_swapChain->GetLightingResultImageView();
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_compositeDescriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);

    VkDescriptorSetAllocateInfo emissiveAllocInfo{};
    emissiveAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    emissiveAllocInfo.descriptorPool = m_descriptorPool;
    emissiveAllocInfo.descriptorSetCount = 1;
    emissiveAllocInfo.pSetLayouts = &m_compositeSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &emissiveAllocInfo, &m_emissiveAccumDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate composite descriptor set (emissive accum)!");

    VkDescriptorImageInfo emissiveImageInfo{};
    emissiveImageInfo.sampler = m_dummySampler;
    emissiveImageInfo.imageView = m_swapChain->GetEmissiveAccumImageView();
    emissiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet emissiveWrite{};
    emissiveWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    emissiveWrite.pNext = nullptr;
    emissiveWrite.dstSet = m_emissiveAccumDescriptorSet;
    emissiveWrite.dstBinding = 0;
    emissiveWrite.dstArrayElement = 0;
    emissiveWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    emissiveWrite.descriptorCount = 1;
    emissiveWrite.pImageInfo = &emissiveImageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &emissiveWrite, 0, nullptr);
}

void DescriptorsManager::CreateHdrDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_compositeSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_hdrDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate HDR descriptor set!");

    UpdateHdrDescriptorSet();
}

void DescriptorsManager::UpdateHdrDescriptorSet()
{
    if (m_hdrDescriptorSet == VK_NULL_HANDLE) return;

    VkImageView hdrView = m_swapChain->GetHdrColorImageView();
    if (hdrView == VK_NULL_HANDLE)
        hdrView = m_dummyImageView;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_dummySampler;
    imageInfo.imageView = hdrView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstSet = m_hdrDescriptorSet;
    write.dstBinding = 0;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);
}

void DescriptorsManager::UpdateGBufferDescriptorSet()
{
    if (m_gbufferDescriptorSet == VK_NULL_HANDLE) return;

    VkImageView gBufferViews[5] = {
        m_swapChain->GetPositionImageView(),
        m_swapChain->GetNormalImageView(),
        m_swapChain->GetAlbedoImageView(),
        m_swapChain->GetMaterialImageView(),
        m_swapChain->GetDepthImageView()
    };

    VkDescriptorImageInfo imageInfos[5]{};
    VkWriteDescriptorSet writes[5]{};
    for (uint32_t i = 0; i < 5; i++)
    {
        imageInfos[i].sampler = m_dummySampler;
        imageInfos[i].imageView = gBufferViews[i];
        imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writes[i].pNext = nullptr;
        writes[i].dstSet = m_gbufferDescriptorSet;
        writes[i].dstBinding = i;
        writes[i].dstArrayElement = 0;
        writes[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        writes[i].descriptorCount = 1;
        writes[i].pImageInfo = &imageInfos[i];
    }

    vkUpdateDescriptorSets(m_device->GetDevice(), 5, writes, 0, nullptr);
}

void DescriptorsManager::CreateLuminanceSetLayout()
{
    std::array<VkDescriptorSetLayoutBinding, 2> bindings{};

    bindings[0].binding = 0;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    bindings[0].descriptorCount = 1;
    bindings[0].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    bindings[1].binding = 1;
    bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    bindings[1].descriptorCount = 1;
    bindings[1].stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
    layoutInfo.pBindings = bindings.data();

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &layoutInfo, nullptr, &m_luminanceSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create luminance descriptor set layout!");
}

void DescriptorsManager::CreateLuminanceDescriptorSet()
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &m_luminanceSetLayout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_luminanceDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate luminance descriptor set!");

    UpdateLuminanceDescriptorSet();
}

void DescriptorsManager::UpdateLuminanceDescriptorSet()
{
    if (m_luminanceDescriptorSet == VK_NULL_HANDLE) return;

    VkImageView hdrView = m_swapChain->GetHdrColorImageView();
    if (hdrView == VK_NULL_HANDLE)
        hdrView = m_dummyImageView;

    VkDescriptorImageInfo imageInfo{};
    imageInfo.sampler = m_dummySampler;
    imageInfo.imageView = hdrView;
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkDescriptorBufferInfo luminanceInfo{};
    luminanceInfo.buffer = m_resourceManager->GetLuminanceStorageBuffer();
    luminanceInfo.offset = 0;
    luminanceInfo.range = VK_WHOLE_SIZE;

    std::array<VkWriteDescriptorSet, 2> writes{};

    writes[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[0].dstSet = m_luminanceDescriptorSet;
    writes[0].dstBinding = 0;
    writes[0].descriptorCount = 1;
    writes[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    writes[0].pImageInfo = &imageInfo;

    writes[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writes[1].dstSet = m_luminanceDescriptorSet;
    writes[1].dstBinding = 1;
    writes[1].descriptorCount = 1;
    writes[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    writes[1].pBufferInfo = &luminanceInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void DescriptorsManager::UpdateCompositeDescriptorSet()
{
    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;

    if (m_compositeDescriptorSet != VK_NULL_HANDLE)
    {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.sampler = m_dummySampler;
        imageInfo.imageView = m_swapChain->GetLightingResultImageView();
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        write.dstSet = m_compositeDescriptorSet;
        write.dstBinding = 0;
        write.pImageInfo = &imageInfo;
        vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);
    }

    if (m_emissiveAccumDescriptorSet != VK_NULL_HANDLE)
    {
        VkDescriptorImageInfo emissiveImageInfo{};
        emissiveImageInfo.sampler = m_dummySampler;
        emissiveImageInfo.imageView = m_swapChain->GetEmissiveAccumImageView();
        emissiveImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        write.dstSet = m_emissiveAccumDescriptorSet;
        write.dstBinding = 0;
        write.pImageInfo = &emissiveImageInfo;
        vkUpdateDescriptorSets(m_device->GetDevice(), 1, &write, 0, nullptr);
    }
}