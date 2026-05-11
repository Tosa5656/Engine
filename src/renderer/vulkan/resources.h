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

static const uint32_t MAX_LIGHTS = 1024;

struct PerFrameUBO
{
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
    alignas(16) glm::vec3 cameraPos;
    alignas(4) float nearPlane;
    alignas(4) float farPlane;
    alignas(4) float exposure;
    alignas(4) float pad;
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
    alignas(4) float alphaCutoff;
    alignas(4) int alphaMode;
};

struct ClusterGridBuffer
{
    alignas(4) uint32_t tileCountX;
    alignas(4) uint32_t tileCountY;
    alignas(4) uint32_t clusterCount;
    alignas(4) uint32_t depthSlices;
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
    void SetExposure(float exposure) { m_exposure = exposure; }
    float GetExposure() const { return m_exposure; }
    uint32_t AllocateObjectSlot();
    void FreeObjectSlot(uint32_t slot);

    VkDevice GetVkDevice();

    std::vector<VkBuffer>& GetPerFrameBuffers();
    std::vector<VmaAllocation> GetPerFrameBufferAllocations();
    VkBuffer GetObjectBuffer() const { return m_objectBuffer; }
    VmaAllocation GetObjectBufferAllocation() const { return m_objectAllocation; }
    uint32_t GetObjectUBOStride() const { return m_objectUBOStride; }
    VmaAllocator GetAllocator();

    void CreateLightBuffers();
    void UpdateLightBuffer(const std::vector<LightUBO>& lights, int lightCount);
    VkBuffer GetLightSSBO() const { return m_lightSSBO; }
    VkDeviceSize GetLightBufferSize() const { return m_lightBufferSize; }
    VkBuffer GetClusterCountSSBO() const { return m_clusterCountSSBO; }
    VkBuffer GetClusterIndexSSBO() const { return m_clusterIndexSSBO; }
    VkBuffer GetClusterGridInfoUBO() const { return m_clusterGridInfoUBO; }

    void CreateClusterGrid(uint32_t tileCountX, uint32_t tileCountY, uint32_t depthSlices);
    void UpdateClusterGridInfo(const ClusterGridBuffer& info);

    uint32_t GetClusterCount() const { return m_clusterCount; }

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

    VkBuffer m_lightSSBO = VK_NULL_HANDLE;
    VmaAllocation m_lightAllocation = VK_NULL_HANDLE;
    VkDeviceSize m_lightBufferSize = 0;

    VkBuffer m_clusterCountSSBO = VK_NULL_HANDLE;
    VmaAllocation m_clusterCountAllocation = VK_NULL_HANDLE;
    VkBuffer m_clusterIndexSSBO = VK_NULL_HANDLE;
    VmaAllocation m_clusterIndexAllocation = VK_NULL_HANDLE;
    VkBuffer m_clusterGridInfoUBO = VK_NULL_HANDLE;
    VmaAllocation m_clusterGridInfoAllocation = VK_NULL_HANDLE;

    uint32_t m_clusterCount = 0;
    static const uint32_t MAX_LIGHTS_PER_CLUSTER = 64;
    float m_exposure = 1.0f;
};
