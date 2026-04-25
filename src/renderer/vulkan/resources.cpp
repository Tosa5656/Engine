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

void ResourceManager::UpdateUniformBuffer(uint32_t currentImage, const glm::mat4& model, Camera& camera, const glm::vec3& orbitTarget, float orbitDistance, float orbitYaw, float orbitPitch)
{
    float cosPitch = glm::cos(orbitPitch);
    float sinPitch = glm::sin(orbitPitch);
    float cosYaw = glm::cos(orbitYaw);
    float sinYaw = glm::sin(orbitYaw);

    glm::vec3 direction(
        cosPitch * sinYaw,
        sinPitch,
        cosPitch * cosYaw
    );

    glm::vec3 cameraPos = orbitTarget + direction * orbitDistance;
    camera.SetPosition(cameraPos);
    camera.SetTarget(orbitTarget);

    UniformBufferObject ubo{};
    ubo.model = model;
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();
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
