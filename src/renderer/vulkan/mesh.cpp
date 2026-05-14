#include "mesh.h"

#include <cstring>
#include <utils/parsers/obj.h>

Mesh::Mesh() {};
Mesh::~Mesh() { Destroy(); }

void Mesh::SetDeviceAndAllocator(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator)
{
    m_device = device;
    m_commandBufferManager = cmdManager;
    m_allocator = allocator;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    if (m_uploadFence == VK_NULL_HANDLE && device)
        vkCreateFence(device->GetDevice(), &fenceInfo, nullptr, &m_uploadFence);
}

void Mesh::Init(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, const std::vector<MeshVertex>& vertices, const std::vector<MeshIndex>& indices)
{
    SetDeviceAndAllocator(device, cmdManager, allocator);

    m_vertices = vertices;
    m_indices = indices;

    if (m_vertices.empty() || m_indices.empty())
        return;

    UploadToBuffers();
}

bool Mesh::LoadFromFile(const std::string& filename)
{
    if (!ObjParser::Load(filename, m_vertices, m_indices))
        return false;

    if (m_vertices.empty() || m_indices.empty())
        return false;

    if (m_device == nullptr || m_allocator == VK_NULL_HANDLE)
        return false;

    UploadToBuffers();
    return true;
}

bool Mesh::LoadFromFile(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, const std::string& filename)
{
    SetDeviceAndAllocator(device, cmdManager, allocator);
    return LoadFromFile(filename);
}

void Mesh::UploadToBuffers()
{
    Destroy();

    VkDeviceSize vertexBufferSize = sizeof(m_vertices[0]) * m_vertices.size();
    VkBuffer stagingBuffer;
    VmaAllocation stagingAllocation;
    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation);

    void* data;
    vmaMapMemory(m_allocator, stagingAllocation, &data);
    memcpy(data, m_vertices.data(), static_cast<size_t>(vertexBufferSize));
    vmaUnmapMemory(m_allocator, stagingAllocation);

    CreateBuffer(vertexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_vertexBuffer, m_vertexBufferAllocation);
    CopyBuffer(stagingBuffer, m_vertexBuffer, vertexBufferSize);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);

    VkDeviceSize indexBufferSize = sizeof(m_indices[0]) * m_indices.size();
    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingAllocation);

    vmaMapMemory(m_allocator, stagingAllocation, &data);
    memcpy(data, m_indices.data(), static_cast<size_t>(indexBufferSize));
    vmaUnmapMemory(m_allocator, stagingAllocation);

    CreateBuffer(indexBufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY, m_indexBuffer, m_indexBufferAllocation);
    CopyBuffer(stagingBuffer, m_indexBuffer, indexBufferSize);
    vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);
}

void Mesh::Draw(VkCommandBuffer commandBuffer)
{
    if (m_vertexBuffer == VK_NULL_HANDLE || m_indexBuffer == VK_NULL_HANDLE || m_indices.empty())
        return;

    VkBuffer vertexBuffers[] = { m_vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, m_indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(m_indices.size()), 1, 0, 0, 0);
}

void Mesh::Destroy()
{
    if (m_allocator == VK_NULL_HANDLE)
        return;

    if (m_vertexBuffer != VK_NULL_HANDLE && m_vertexBufferAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_vertexBuffer, m_vertexBufferAllocation);
        m_vertexBuffer = VK_NULL_HANDLE;
        m_vertexBufferAllocation = VK_NULL_HANDLE;
    }

    if (m_indexBuffer != VK_NULL_HANDLE && m_indexBufferAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyBuffer(m_allocator, m_indexBuffer, m_indexBufferAllocation);
        m_indexBuffer = VK_NULL_HANDLE;
        m_indexBufferAllocation = VK_NULL_HANDLE;
    }
}

void Mesh::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation)
{
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = memoryUsage;

    if (vmaCreateBuffer(m_allocator, &bufferInfo, &allocInfo, &buffer, &allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create buffer!");
}

void Mesh::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_commandBufferManager->GetCommandPool();
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    if (vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, &commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate command buffer for copy!");
    }

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

    vkResetFences(m_device->GetDevice(), 1, &m_uploadFence);
    vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, m_uploadFence);
    vkWaitForFences(m_device->GetDevice(), 1, &m_uploadFence, VK_TRUE, UINT64_MAX);

    vkFreeCommandBuffers(m_device->GetDevice(), m_commandBufferManager->GetCommandPool(), 1, &commandBuffer);
}