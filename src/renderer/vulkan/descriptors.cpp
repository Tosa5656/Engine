#include "descriptors.h"
#include "resources.h"

DescriptorsManager::DescriptorsManager() {}
DescriptorsManager::~DescriptorsManager() {}

void DescriptorsManager::Init(Device* device, SwapChain* swapChain, ResourceManager* resourceManager, uint32_t maxObjects)
{
    m_device = device;
    m_swapChain = swapChain;
    m_resourceManager = resourceManager;
}

void DescriptorsManager::Cleanup()
{
    if (m_descriptorPool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(m_device->GetDevice(), m_descriptorPool, nullptr);
        m_descriptorPool = VK_NULL_HANDLE;
    }

    if (m_perFrameSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_perFrameSetLayout, nullptr);
        m_perFrameSetLayout = VK_NULL_HANDLE;
    }

    if (m_perObjectSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_perObjectSetLayout, nullptr);
        m_perObjectSetLayout = VK_NULL_HANDLE;
    }

    if (m_computeSetLayout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(m_device->GetDevice(), m_computeSetLayout, nullptr);
        m_computeSetLayout = VK_NULL_HANDLE;
    }
}

void DescriptorsManager::CreateDescriptorPool()
{
    std::vector<VkDescriptorPoolSize> poolSizes(3);
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size());
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
    poolSizes[1].descriptorCount = 256;
    poolSizes[2].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[2].descriptorCount = 1;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    poolInfo.maxSets = static_cast<uint32_t>(m_swapChain->GetSwapChainImages().size()) + 2;

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