#include "object.h"

Object::Object()
{
}

Object::~Object()
{
    Destroy();
}

void Object::Init(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator, ResourceManager* resourceManager, std::string model_path)
{
    m_device = device;
    m_commandBufferManager = cmdManager;
    m_allocator = allocator;

    m_mesh.SetDeviceAndAllocator(device, cmdManager, allocator);
    m_mesh.LoadFromFile(model_path);

    m_uboSlot = resourceManager->AllocateObjectSlot();
}

void Object::Draw(VkCommandBuffer commandBuffer, VkDescriptorSet perObjectDescriptorSet, uint32_t objectStride)
{
    m_mesh.Draw(commandBuffer);
}

void Object::Destroy()
{
    m_mesh.Destroy();
}

Mesh* Object::GetMesh()
{
    return &m_mesh;
}

Transform* Object::GetTransform()
{
    return &m_transform;
}

void Object::SetDeviceAndAllocator(Device* device, CommandBufferManager* cmdManager, VmaAllocator allocator)
{
    m_device = device;
    m_commandBufferManager = cmdManager;
    m_allocator = allocator;
}

void Object::UpdateUBO(ResourceManager* resourceManager, const glm::vec3& color, float roughness)
{
    resourceManager->UpdatePerObjectUBO(m_uboSlot, m_transform.GetTransformationMatrix(), color, roughness);
}