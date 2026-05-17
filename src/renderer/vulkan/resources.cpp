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
    if (m_allocator == VK_NULL_HANDLE)
        return;

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

    if (m_lightSSBO != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_lightSSBO, m_lightAllocation);
        m_lightSSBO = VK_NULL_HANDLE;
    }

    if (m_clusterCountSSBO != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_clusterCountSSBO, m_clusterCountAllocation);
        m_clusterCountSSBO = VK_NULL_HANDLE;
    }

    if (m_clusterIndexSSBO != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_clusterIndexSSBO, m_clusterIndexAllocation);
        m_clusterIndexSSBO = VK_NULL_HANDLE;
    }

    if (m_clusterGridInfoUBO != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_clusterGridInfoUBO, m_clusterGridInfoAllocation);
        m_clusterGridInfoUBO = VK_NULL_HANDLE;
    }

    if (m_luminanceStorageBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_luminanceStorageBuffer, m_luminanceStorageAllocation);
        m_luminanceStorageBuffer = VK_NULL_HANDLE;
    }

    if (m_luminanceStagingBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_luminanceStagingBuffer, m_luminanceStagingAllocation);
        m_luminanceStagingBuffer = VK_NULL_HANDLE;
    }

    if (m_pointShadowMatrixBuffer != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_pointShadowMatrixBuffer, m_pointShadowMatrixAllocation);
        m_pointShadowMatrixBuffer = VK_NULL_HANDLE;
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

void ResourceManager::UpdatePerFrameUBO(uint32_t currentImage, Camera& camera, const glm::mat4 lightSpaceMatrices[3], const glm::vec4& cascadeSplits, int pcfKernel, float normalOffsetBias, float depthBias, const float cascadeUVScale[3], bool debugCascades, float pcssLightSize, float pcssBlockerRadius)
{
    PerFrameUBO ubo{};
    ubo.view = camera.GetViewMatrix();
    ubo.proj = camera.GetProjectionMatrix();
    ubo.cameraPos = camera.GetPosition();
    ubo.nearPlane = camera.GetNearPlane();
    ubo.farPlane = camera.GetFarPlane();
    ubo.exposure = m_exposure;
    ubo.lightSpaceMatrix = lightSpaceMatrices[0];
    ubo.lightSpaceMatrices[0] = lightSpaceMatrices[1];
    ubo.lightSpaceMatrices[1] = lightSpaceMatrices[2];
    ubo.cascadeSplits = cascadeSplits;
    ubo.pcfKernel = pcfKernel;
    ubo.normalOffsetBias = normalOffsetBias;
    ubo.depthBias = depthBias;
    ubo.cascadeUVScale0 = cascadeUVScale[0];
    ubo.cascadeUVScale1 = cascadeUVScale[1];
    ubo.cascadeUVScale2 = cascadeUVScale[2];
    ubo.debugCascades = debugCascades ? 1 : 0;
    ubo.pcssLightSize = pcssLightSize;
    ubo.pcssBlockerRadius = pcssBlockerRadius;

    void* data;
    vmaMapMemory(m_allocator, m_perFrameAllocations[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(m_allocator, m_perFrameAllocations[currentImage]);
}

void ResourceManager::CreateLightBuffers()
{
    VkDeviceSize bufferSize = sizeof(LightUBO) * MAX_LIGHTS + sizeof(int);
    m_lightBufferSize = bufferSize;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_lightSSBO, m_lightAllocation);
}

void ResourceManager::UpdateLightBuffer(const std::vector<LightUBO>& lights, int lightCount)
{
    VkDeviceSize bufferSize = sizeof(LightUBO) * MAX_LIGHTS + sizeof(int);

    void* mappedData;
    vmaMapMemory(m_allocator, m_lightAllocation, &mappedData);
    memset(mappedData, 0, bufferSize);
    if (lightCount > 0)
        memcpy(mappedData, lights.data(), sizeof(LightUBO) * std::min(lightCount, (int)MAX_LIGHTS));
    memcpy(static_cast<char*>(mappedData) + sizeof(LightUBO) * MAX_LIGHTS, &lightCount, sizeof(int));
    vmaUnmapMemory(m_allocator, m_lightAllocation);
}

void ResourceManager::CreatePointShadowMatrixBuffer()
{
    m_pointShadowMatrixBufferSize = MAX_POINT_SHADOWS * 6 * sizeof(glm::mat4);
    CreateBuffer(m_pointShadowMatrixBufferSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_pointShadowMatrixBuffer, m_pointShadowMatrixAllocation);
}

void ResourceManager::UpdatePointShadowMatrices(const glm::mat4* matrices, int count)
{
    VkDeviceSize uploadSize = sizeof(glm::mat4) * std::min(count, (int)(MAX_POINT_SHADOWS * 6));
    void* data;
    vmaMapMemory(m_allocator, m_pointShadowMatrixAllocation, &data);
    memset(data, 0, m_pointShadowMatrixBufferSize);
    memcpy(data, matrices, uploadSize);
    vmaUnmapMemory(m_allocator, m_pointShadowMatrixAllocation);
}

void ResourceManager::CreateClusterGrid(uint32_t tileCountX, uint32_t tileCountY, uint32_t depthSlices)
{
    m_clusterCount = tileCountX * tileCountY * depthSlices;

    VkDeviceSize countSize = sizeof(uint32_t) * m_clusterCount;
    VkDeviceSize indexSize = sizeof(uint32_t) * m_clusterCount * MAX_LIGHTS_PER_CLUSTER;

    if (m_clusterCountSSBO != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_allocator, m_clusterCountSSBO, m_clusterCountAllocation);
    if (m_clusterIndexSSBO != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_allocator, m_clusterIndexSSBO, m_clusterIndexAllocation);

    CreateBuffer(countSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_clusterCountSSBO, m_clusterCountAllocation);
    CreateBuffer(indexSize, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_clusterIndexSSBO, m_clusterIndexAllocation);

    if (m_clusterGridInfoUBO == VK_NULL_HANDLE)
    {
        CreateBuffer(sizeof(ClusterGridBuffer), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, m_clusterGridInfoUBO, m_clusterGridInfoAllocation);
    }

    ClusterGridBuffer info{};
    info.tileCountX = tileCountX;
    info.tileCountY = tileCountY;
    info.clusterCount = m_clusterCount;
    info.depthSlices = depthSlices;
    UpdateClusterGridInfo(info);
}

void ResourceManager::UpdateClusterGridInfo(const ClusterGridBuffer& info)
{
    void* data;
    vmaMapMemory(m_allocator, m_clusterGridInfoAllocation, &data);
    memcpy(data, &info, sizeof(info));
    vmaUnmapMemory(m_allocator, m_clusterGridInfoAllocation);
}

void ResourceManager::CreateLuminanceBuffers()
{
    VkDeviceSize bufferSize = sizeof(uint32_t) * 256;

    CreateBuffer(bufferSize,
        VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VMA_MEMORY_USAGE_GPU_ONLY, m_luminanceStorageBuffer, m_luminanceStorageAllocation);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
        VMA_MEMORY_USAGE_GPU_TO_CPU, m_luminanceStagingBuffer, m_luminanceStagingAllocation);
}

void ResourceManager::ReadLuminanceHistogram(uint32_t* outHistogram)
{
    void* mappedData;
    vmaMapMemory(m_allocator, m_luminanceStagingAllocation, &mappedData);
    memcpy(outHistogram, mappedData, sizeof(uint32_t) * 256);
    vmaUnmapMemory(m_allocator, m_luminanceStagingAllocation);
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

void ResourceManager::FreeObjectSlot(uint32_t slot)
{
    m_freeSlots.push_back(slot);
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

std::vector<VmaAllocation>& ResourceManager::GetPerFrameBufferAllocations()
{
    return m_perFrameAllocations;
}

VmaAllocator ResourceManager::GetAllocator()
{
    return m_allocator;
}

void ResourceManager::GetMemoryBudget(uint64_t& gpuMemoryUsed, uint64_t& gpuMemoryBudget)
{
    VmaBudget budgets[VK_MAX_MEMORY_HEAPS];
    vmaGetHeapBudgets(m_allocator, budgets);
    
    gpuMemoryUsed = 0;
    gpuMemoryBudget = 0;
    
    for (int i = 0; i < VK_MAX_MEMORY_HEAPS; i++)
    {
        if (m_device->IsMemoryHeapDeviceLocal(i))
        {
            gpuMemoryUsed += budgets[i].usage;
            gpuMemoryBudget += budgets[i].budget;
        }
    }
}

VkDevice ResourceManager::GetVkDevice()
{
    return m_device->GetDevice();
}
