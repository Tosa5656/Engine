#include "resources.h"

#include "descriptors.h"

ResourceManager::ResourceManager() {}
ResourceManager::~ResourceManager() {}

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

void ResourceManager::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size, CommandBufferManager* commandBufferManager, Device* device)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandBufferManager->GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(device->GetGraphicsQueue());

    vkFreeCommandBuffers(device->GetDevice(), commandBufferManager->GetCommandPool(), 1, &commandBuffer);
}

void ResourceManager::CreateVertexBuffer(CommandBufferManager* commandBufferManager, Device* device)
{
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation);

    void* data;
    vmaMapMemory(m_allocator, stagingAllocation, &data);
    memcpy(data, vertices.data(), (size_t)bufferSize);
    vmaUnmapMemory(m_allocator, stagingAllocation);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_vertexBuffer, m_vertexBufferAllocation);
    CopyBuffer(stagingBuffer, m_vertexBuffer, bufferSize, commandBufferManager, device);

    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);
}

void ResourceManager::CreateIndexBuffer(CommandBufferManager* commandBufferManager, Device* device)
{
    VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation);

    void* data;
    vmaMapMemory(m_allocator, stagingAllocation, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vmaUnmapMemory(m_allocator, stagingAllocation);

    CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_indexBuffer, m_indexBufferAllocation);

    CopyBuffer(stagingBuffer, m_indexBuffer, bufferSize, commandBufferManager, device);

    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);
}

void ResourceManager::CreateUniformBuffers(DescriptorsManager* descriptorManager, SwapChain* swapChain)
{
    VkDeviceSize bufferSize = sizeof(UniformBufferObject);

    auto& uniformBuffers = descriptorManager->GetUniformBuffers();
    uniformBuffers.resize(swapChain->GetSwapChainImages().size());
    m_uniformAllocations.resize(swapChain->GetSwapChainImages().size());

    for (size_t i = 0; i < swapChain->GetSwapChainImages().size(); i++)
    {
        CreateBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU, descriptorManager->GetUniformBuffers()[i], m_uniformAllocations[i]);
    }
}

void ResourceManager::UpdateUniformBuffer(uint32_t currentImage, SwapChain* swapChain)
{
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

    UniformBufferObject ubo{};
    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ubo.proj = glm::perspective(glm::radians(45.0f), swapChain->GetSwapChainExtent().width / (float) swapChain->GetSwapChainExtent().height, 0.1f, 10.0f);
    ubo.proj[1][1] *= -1;

    void* data;
    vmaMapMemory(m_allocator, m_uniformAllocations[currentImage], &data);
    memcpy(data, &ubo, sizeof(ubo));
    vmaUnmapMemory(m_allocator, m_uniformAllocations[currentImage]);
}

void ResourceManager::CreateAllocator(Device* device, Instance* instance)
{
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = device->GetPhysicalDevice();
    allocatorInfo.device = device->GetDevice();
    allocatorInfo.instance = instance->GetInstance();
    allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_4;

    if (vmaCreateAllocator(&allocatorInfo, &m_allocator) != VK_SUCCESS)
        throw std::runtime_error("failed to create VMA allocator!");
}

void ResourceManager::Cleanup(DescriptorsManager* descriptorManager)
{
    auto& uniformBuffers = descriptorManager->GetUniformBuffers();
    for (size_t i = 0; i < m_uniformAllocations.size(); i++)
    {
        if (uniformBuffers[i] != VK_NULL_HANDLE)
            vmaDestroyBuffer(m_allocator, uniformBuffers[i], m_uniformAllocations[i]);
    }
    uniformBuffers.clear();
    m_uniformAllocations.clear();

    if (m_vertexBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_allocator, m_vertexBuffer, m_vertexBufferAllocation);
    if (m_indexBuffer != VK_NULL_HANDLE)
        vmaDestroyBuffer(m_allocator, m_indexBuffer, m_indexBufferAllocation);
}

VkBuffer ResourceManager::GetVertexBuffer()
{
    return m_vertexBuffer;
}

VmaAllocation ResourceManager::GetVertexBufferAllocation()
{
    return m_vertexBufferAllocation;
}

VkBuffer ResourceManager::GetIndexBuffer()
{
    return m_indexBuffer;
}

VmaAllocation ResourceManager::GetIndexBufferAllocation()
{
    return m_indexBufferAllocation;
}

std::vector<VmaAllocation> ResourceManager::GetUniformBufferAllocation()
{
    return m_uniformAllocations;
}

VmaAllocator ResourceManager::GetAllocator()
{
    return m_allocator;
}
