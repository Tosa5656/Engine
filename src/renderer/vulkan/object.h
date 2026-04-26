#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <renderer/vulkan/mesh.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/swapchain.h>
#include <renderer/vulkan/resources.h>
#include <utils/vars/transform.h>

class Object
{
public:
    Object();
    ~Object();

    void Init(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, ResourceManager* resourceManager, std::string model_path);
    void Draw(VkCommandBuffer commandBuffer, VkDescriptorSet perObjectDescriptorSet, uint32_t objectStride);
    void Destroy();

    Mesh* GetMesh();
    Transform* GetTransform();
    uint32_t GetUBOSlot() const { return m_uboSlot; }

    void SetDeviceAndAllocator(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator);
    void UpdateUBO(ResourceManager* resourceManager, const glm::vec3& color, float roughness);

private:
    Device* m_device = nullptr;
    CommandBufferManager* m_commandBufferManager = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    uint32_t m_uboSlot = 0;

    Mesh m_mesh;
    Transform m_transform = Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
};