#pragma once

#include <vector>
#include <string>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>
#include <renderer/vulkan/commandbuffer.h>

struct MeshVertex
{
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    glm::vec3 tangent;

    bool operator==(const MeshVertex& other) const noexcept
    {
        return pos == other.pos && normal == other.normal && uv == other.uv && tangent == other.tangent;
    }

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(MeshVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};

        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(MeshVertex, pos);

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(MeshVertex, normal);

        attributeDescriptions[2].binding  = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format   = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset   = offsetof(MeshVertex, uv);

        attributeDescriptions[3].binding  = 0;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[3].offset   = offsetof(MeshVertex, tangent);

        return attributeDescriptions;
    }
};

struct MeshIndex
{
    uint16_t value;
};

class Mesh
{
public:
    Mesh();
    ~Mesh();

    void Init(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, const std::vector<MeshVertex>& vertices, const std::vector<MeshIndex>& indices);
    void Draw(VkCommandBuffer commandBuffer);
    void Destroy();

    VkBuffer GetVertexBuffer() const { return m_vertexBuffer; }
    VmaAllocation GetVertexBufferAllocation() const { return m_vertexBufferAllocation; }
    VkBuffer GetIndexBuffer() const { return m_indexBuffer; }
    VmaAllocation GetIndexBufferAllocation() const { return m_indexBufferAllocation; }
    uint32_t GetIndexCount() const { return static_cast<uint32_t>(m_indices.size()); }
    CommandBufferManager* GetCommandBufferManager() const { return m_commandBufferManager; }

    bool LoadFromFile(const std::string& filename);
    bool LoadFromFile(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, const std::string& filename);

    void SetDeviceAndAllocator(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator);

private:
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void UploadToBuffers();

    Device* m_device = nullptr;
    CommandBufferManager* m_commandBufferManager = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    std::vector<MeshVertex> m_vertices;
    std::vector<MeshIndex> m_indices;

    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_vertexBufferAllocation = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VmaAllocation m_indexBufferAllocation = VK_NULL_HANDLE;
    VkFence m_uploadFence = VK_NULL_HANDLE;
};