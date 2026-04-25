#include "resources.h"

#include <cstring>

ResourceManager::ResourceManager() = default;
ResourceManager::~ResourceManager() = default;

void ResourceManager::Create(Device* device, SwapChain* swapChain, Instance* instance)
{
    m_device = device;
    m_swapChain = swapChain;
    m_instance = instance;
}

void ResourceManager::Cleanup()
{
    for (size_t i = 0; i < m_uniformBuffers.size(); i++)
    {
        if (m_uniformBuffers[i] != VK_NULL_HANDLE)
            vmaDestroyBuffer(m_allocator, m_uniformBuffers[i], m_uniformAllocations[i]);
    }
    m_uniformBuffers.clear();
    m_uniformAllocations.clear();
}

void ResourceManager::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create buffer with VMA!");
    }
}

void ResourceManager::CreateUniformBuffers()
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    m_uniformBuffers.resize(m_swapChain->GetSwapChainImages().size());
    m_uniformAllocations.resize(m_swapChain->GetSwapChainImages().size());

    for (size_t i = 0; i < m_swapChain->GetSwapChainImages().size(); i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_uniformBuffers[i], m_uniformAllocations[i]);
    }
}

void ResourceManager::UpdateUniformBuffer(uint32_t currentImage)
{
    static auto startTime = std::chrono::steady_clock::now();
    auto currentTime = std::chrono::steady_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(800.0f, 500.0f, 800.0f), glm::vec3(0.0f, 100.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    ubo.proj = glm::perspective(glm::radians(60.0f), m_swapChain->GetSwapChainExtent().width / (float) m_swapChain->GetSwapChainExtent().height, 0.1f, 2000.0f);
    ubo.proj[1][1] *= -1;
    ubo.color = glm::vec3(0.5f, 0.5f, 0.5f);

    void* data;
    vmaMapMemory(m_allocator, m_uniformAllocations[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(m_allocator, m_uniformAllocations[currentImage]);
}

void ResourceManager::CreateAllocator()
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = m_device->GetPhysicalDevice();
    allocatorInfo.device = m_device->GetDevice();
    allocatorInfo.instance = m_instance->GetInstance();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
        throw std::runtime_error("failed to create VMA allocator!");
}

std::vector<VkBuffer>& ResourceManager::GetUniformBuffers()
{
    return m_uniformBuffers;
}

std::vector<VmaAllocation> ResourceManager::GetUniformBufferAllocation()
{
    return m_uniformAllocations;
}

VmaAllocator ResourceManager::GetAllocator()
{
    return m_allocator;
}
