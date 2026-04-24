#pragma once

#include <vector>
#include <cstddef>
#include <chrono>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/swapchain.h>

struct UniformBufferObject
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 color;
};

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void Create(Device* device, SwapChain* swapChain, Instance* instance);
    void Cleanup();

    void CreateUniformBuffers();
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation);
    void CreateAllocator();

    void UpdateUniformBuffer(uint32_t currentImage);

    std::vector<VkBuffer>& GetUniformBuffers();
    std::vector<VmaAllocation> GetUniformBufferAllocation();
    VmaAllocator GetAllocator();
private:
    Device* m_device;
    SwapChain* m_swapChain;
    Instance* m_instance;

    std::vector<VkBuffer> m_uniformBuffers;
    std::vector<VmaAllocation> m_uniformAllocations;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};