#include "descriptors.h"
#include "resources.h"

DescriptorsManager::DescriptorsManager() {}
DescriptorsManager::~DescriptorsManager() {}

void DescriptorsManager::CreateDescriptorPool(Device* device, SwapChain* swapChain)
{
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(swapChain->GetSwapChainImages().size());

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(swapChain->GetSwapChainImages().size());

    if (vkCreateDescriptorPool(device->GetDevice(), &poolInfo, nullptr, &m_descriptorPool) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor pool!");
    }
}

void DescriptorsManager::CreateDescriptorSets(Device* device, SwapChain* swapChain)
{
    std::vector<VkDescriptorSetLayout> layouts(swapChain->GetSwapChainImages().size(), m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(swapChain->GetSwapChainImages().size());
    allocInfo.pSetLayouts = layouts.data();

    m_descriptorSets.resize(swapChain->GetSwapChainImages().size());

    if (vkAllocateDescriptorSets(device->GetDevice(), &allocInfo, m_descriptorSets.data()) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < swapChain->GetSwapChainImages().size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = m_uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device->GetDevice(), 1, &descriptorWrite, 0, nullptr);
    }
}

void DescriptorsManager::CreateDescriptorSetLayout(Device* device)
{
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device->GetDevice(), &layoutInfo, nullptr, &m_descriptorSetLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create descriptor set layout!");
    }
}

VkDescriptorPool DescriptorsManager::GetDescriptorPool()
{
    return m_descriptorPool;
}

std::vector<VkDescriptorSet> DescriptorsManager::GetDescriptorSets()
{
    return m_descriptorSets;
}

VkDescriptorSetLayout DescriptorsManager::GetDescriptorSetLayout()
{
    return m_descriptorSetLayout;
}

std::vector<VkBuffer>& DescriptorsManager::GetUniformBuffers()
{
    return m_uniformBuffers;
}
