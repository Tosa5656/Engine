#include "descriptors.h"
#include "resources.h"

DescriptorsManager::DescriptorsManager() : m_dummySampler(VK_NULL_HANDLE), m_dummyImageView(VK_NULL_HANDLE), m_dummyImageMemory(VK_NULL_HANDLE) {}
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

    if (m_computeSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_computeSetLayout, nullptr);
        m_computeSetLayout = VK_NULL_HANDLE;
    }

    if (m_textureSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device, m_textureSetLayout, nullptr);
        m_textureSetLayout = VK_NULL_HANDLE;
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
}

void DescriptorsManager::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(4);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = 256;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = 1;
    poolSizes[3].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[3].descriptorCount = 256;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size()) + 2 + 256;

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
    memoryAllocInfo.memoryTypeIndex = 0;

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

void DescriptorsManager::CreateComputeDescriptorSetLayout()
{
    VkDescriptorSetLayoutBinding computeBinding{};
    computeBinding.binding = 0;
    computeBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    computeBinding.descriptorCount = 1;
    computeBinding.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;

    VkDescriptorSetLayoutCreateInfo computeLayoutInfo{};
    computeLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    computeLayoutInfo.bindingCount = 1;
    computeLayoutInfo.pBindings = &computeBinding;

    if (vkCreateDescriptorSetLayout(m_device->GetDevice(), &computeLayoutInfo, nullptr, &m_computeSetLayout) != VK_SUCCESS)
        throw std::runtime_error("failed to create compute descriptor set layout!");
}

void DescriptorsManager::CreateComputeDescriptorSet()
{
    VkDescriptorSetLayout layout = m_computeSetLayout;

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &layout;

    if (vkAllocateDescriptorSets(m_device->GetDevice(), &allocInfo, &m_computeDescriptorSet) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate compute descriptor set!");

    // Use GPU buffer for compute
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = m_resourceManager->GetComputeResultBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = sizeof(float);

    VkWriteDescriptorSet descriptorWrite{};
    descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWrite.dstSet = m_computeDescriptorSet;
    descriptorWrite.dstBinding = 0;
    descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    descriptorWrite.descriptorCount = 1;
    descriptorWrite.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(m_device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
}

VkDescriptorSetLayout DescriptorsManager::GetComputeDescriptorSetLayout()
{
    return m_computeSetLayout;
}

VkDescriptorSet DescriptorsManager::GetComputeDescriptorSet()
{
    return m_computeDescriptorSet;
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