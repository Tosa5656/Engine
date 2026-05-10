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
#include <renderer/vulkan/camera.h>

struct PerFrameUBO
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 cameraPos;
    float padding;
};

struct LightUBO
{
    alignas(16) glm::vec4 position;
    alignas(16) glm::vec4 direction;
    alignas(16) glm::vec4 color;
    alignas(16) glm::vec4 params;
    alignas(16) glm::vec4 atten;
};

enum ParallaxMode : int
{
    ParallaxOcclusionMapping = 0,
    ReliefMapping = 1
};

struct PerObjectUBO
{
    alignas(16) glm::mat4 model;
    alignas(16) glm::vec3 albedo;
    float metallic;
    float roughness;
    float ao;
    float normalStrength;
    alignas(8) glm::vec2 uvOffset;
    glm::vec2 uvScale;
    int parallaxMode;
    float parallaxScale;
    int parallaxIterations;
};

class ResourceManager
{
public:
    ResourceManager();
    ~ResourceManager();

    void Create(Device* device, SwapChain* swapChain, Instance* instance);
    void Cleanup();

    void CreateUniformBuffers();
    void CreateObjectBuffer(uint32_t maxObjects);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage, VkBuffer& buffer, VmaAllocation& allocation);
    void CreateAllocator();

    void UpdatePerFrameUBO(uint32_t currentImage, Camera& camera);
    void UpdatePerObjectUBO(uint32_t slot, const PerObjectUBO& data);
    uint32_t AllocateObjectSlot();
    void FreeObjectSlot(uint32_t slot);

    VkDevice GetVkDevice();

    std::vector<VkBuffer>& GetPerFrameBuffers();
    std::vector<VmaAllocation> GetPerFrameBufferAllocations();
    VkBuffer GetObjectBuffer() const { return m_objectBuffer; }
    VmaAllocation GetObjectBufferAllocation() const { return m_objectAllocation; }
    uint32_t GetObjectUBOStride() const { return m_objectUBOStride; }
    VmaAllocator GetAllocator();

    void CreateLightBuffer(uint32_t maxLights);
    void UpdateLightBuffer(const std::vector<LightUBO>& lights, int lightCount);
    VkBuffer GetLightBuffer() const { return m_lightBuffer; }
    VkDeviceSize GetLightBufferSize() const { return m_lightBufferSize; }

    void CreateComputeResultBuffer();
    VkBuffer GetComputeResultBuffer() const { return m_computeResultBuffer; }
    VmaAllocation GetComputeResultBufferAllocation() const { return m_computeResultAllocation; }
    VkBuffer GetComputeStagingBuffer() const { return m_computeStagingBuffer; }
    VmaAllocation GetComputeStagingBufferAllocation() const { return m_computeStagingAllocation; }
    void ReadComputeResult(float& outResult);

private:
    Device* m_device;
    SwapChain* m_swapChain;
    Instance* m_instance;

    std::vector<VkBuffer> m_perFrameBuffers;
    std::vector<VmaAllocation> m_perFrameAllocations;
    VkBuffer m_objectBuffer = VK_NULL_HANDLE;
    VmaAllocation m_objectAllocation = VK_NULL_HANDLE;
    uint32_t m_objectUBOStride = 0;
    uint32_t m_objectCount = 0;
    std::vector<uint32_t> m_freeSlots;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkBuffer m_lightBuffer = VK_NULL_HANDLE;
    VmaAllocation m_lightAllocation = VK_NULL_HANDLE;
    VkDeviceSize m_lightBufferSize = 0;

    VkBuffer m_computeResultBuffer = VK_NULL_HANDLE;
    VmaAllocation m_computeResultAllocation = VK_NULL_HANDLE;
    VkBuffer m_computeStagingBuffer = VK_NULL_HANDLE;
    VmaAllocation m_computeStagingAllocation = VK_NULL_HANDLE;
};