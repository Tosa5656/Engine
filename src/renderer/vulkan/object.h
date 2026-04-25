#pragma once

#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include <glm/glm.hpp>

#include <renderer/vulkan/mesh.h>
#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/swapchain.h>
#include <utils/vars/transform.h>

class Object
{
public:
    Object();
    ~Object();

    void Init(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, std::string model_path);
    void Draw(VkCommandBuffer commandBuffer);
    void Destroy();

    Mesh* GetMesh();
    Transform* GetTransform();

    void SetDeviceAndAllocator(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator);

private:
    Device* m_device = nullptr;
    CommandBufferManager* m_commandBufferManager = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    Mesh m_mesh;
    Transform m_transform = Transform(glm::vec3(0, 0, 0), glm::vec3(0, 0, 0), glm::vec3(1, 1, 1));
};