#pragma once

#include <stdexcept>
#include <chrono>

#include <vulkan/vulkan.h>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/descriptors.h>
#include <renderer/vulkan/swapchain.h>

struct Vertex
{
    glm::vec3 pos;
    glm::vec3 color;

    static VkVertexInputBindingDescription getBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions()
    {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};

        attributeDescriptions[0].binding  = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset   = offsetof(Vertex, pos);

        attributeDescriptions[1].binding  = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format   = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset   = offsetof(Vertex, color);

        return attributeDescriptions;
    }
};

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void Create(CommandBufferManager* commandBufferManager, Device* device, SwapChain* swapChain, Instance* instance);
    void Cleanup();

    void CreateVertexBuffer();
    void CreateIndexBuffer();
    void CreateUniformBuffers();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation);
    void CreateAllocator();
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void UpdateUniformBuffer(uint32_t currentImage);

    VkBuffer GetVertexBuffer();
    VmaAllocation GetVertexBufferAllocation();
    VkBuffer GetIndexBuffer();
    VmaAllocation GetIndexBufferAllocation();
    std::vector<VkBuffer>& GetUniformBuffers();
    std::vector<VmaAllocation> GetUniformBufferAllocation();
    VmaAllocator GetAllocator();

    // TODO: Create mesh class
    const std::vector<Vertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}}
    };

    const std::vector<uint16_t> indices = {
        0, 1, 2, 2, 3, 0
    };
private:
    CommandBufferManager* m_commandBufferManager;
    Device* m_device;
    SwapChain* m_swapChain;
    Instance* m_instance;

    VkBuffer m_vertexBuffer;
    VmaAllocation m_vertexBufferAllocation = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer;
    VmaAllocation m_indexBufferAllocation = VK_NULL_HANDLE;
    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VmaAllocation> m_uniformAllocations;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};