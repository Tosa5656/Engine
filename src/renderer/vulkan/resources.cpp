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
    for (size_t i = 0; i < m_perFrameBuffers.size(); i++)
    {
        if (m_perFrameBuffers[i] != VK_NULL_HANDLE)
            vmaDestroyBuffer(m_allocator, m_perFrameBuffers[i], m_perFrameAllocations[i]);
    }
    m_perFrameBuffers.clear();
    m_perFrameAllocations.clear();

    if (m_objectBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_objectBuffer, m_objectAllocation);
        m_objectBuffer = VK_NULL_HANDLE;
    }
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
    VkDeviceSize bufferSize = sizeof(PerFrameUBO);

    m_perFrameBuffers.resize(m_swapChain->GetSwapChainImages().size());
    m_perFrameAllocations.resize(m_swapChain->GetSwapChainImages().size());

    for (size_t i = 0; i < m_swapChain->GetSwapChainImages().size(); i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_perFrameBuffers[i], m_perFrameAllocations[i]);
    }
}

void ResourceManager::CreateObjectBuffer(uint32_t maxObjects)
{
    VkPhysicalDevice physicalDevice = m_device->GetPhysicalDevice();
    
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    m_objectUBOStride = static_cast<uint32_t>(sizeof(PerObjectUBO));
    uint32_t alignment = static_cast<uint32_t>(props.limits.minUniformBufferOffsetAlignment);
    if (alignment > 0)
    {
        m_objectUBOStride = ((m_objectUBOStride + alignment - 1) / alignment) * alignment;
    }

    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(m_objectUBOStride) * maxObjects;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_objectBuffer, m_objectAllocation);
}

void ResourceManager::UpdatePerFrameUBO(uint32_t currentImage, Camera& camera, const glm::vec3& orbitTarget, float orbitDistance, float orbitYaw, float orbitPitch)
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

    PerFrameUBO ubo{};
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();

    void* data;
    vmaMapMemory(m_allocator, m_perFrameAllocations[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(m_allocator, m_perFrameAllocations[currentImage]);
}

void ResourceManager::UpdatePerObjectUBO(uint32_t slot, const PerObjectUBO& uboData)
{
    VkDeviceSize offset = static_cast<VkDeviceSize>(slot) * m_objectUBOStride;
    void* mappedData;
    vmaMapMemory(m_allocator, m_objectAllocation, &mappedData);
    char* ptr = static_cast<char*>(mappedData) + offset;
    memcpy(ptr, &uboData, sizeof(PerObjectUBO));
    vmaUnmapMemory(m_allocator, m_objectAllocation);
}

uint32_t ResourceManager::AllocateObjectSlot()
{
    if (!m_freeSlots.empty())
    {
        uint32_t slot = m_freeSlots.back();
        m_freeSlots.pop_back();
        return slot;
    }
    return m_objectCount++;
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

std::vector<VkBuffer>& ResourceManager::GetPerFrameBuffers()
{
    return m_perFrameBuffers;
}

std::vector<VmaAllocation> ResourceManager::GetPerFrameBufferAllocations()
{
    return m_perFrameAllocations;
}

VmaAllocator ResourceManager::GetAllocator()
{
    return m_allocator;
}

VkDevice ResourceManager::GetVkDevice() {
    return m_device->GetDevice();
}
