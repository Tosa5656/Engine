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
    if (m_active)
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

void Object::SetMaterial(Material* material)
{
    m_material = material;
}

void Object::UpdateUBO(ResourceManager* resourceManager)
{
    if (m_material)
    {
        PerObjectUBO uboData = m_material->GetData();
        uboData.model = m_transform.GetTransformationMatrix();
        resourceManager->UpdatePerObjectUBO(m_uboSlot, uboData);
    }
    else
    {
        PerObjectUBO uboData{};
        uboData.model = m_transform.GetTransformationMatrix();
        uboData.albedo = glm::vec3(0.5f, 0.5f, 0.5f);
        uboData.metallic = 0.0f;
        uboData.roughness = 0.5f;
        uboData.ao = 1.0f;
        uboData.normalStrength = 1.0f;
        resourceManager->UpdatePerObjectUBO(m_uboSlot, uboData);
    }
}

void Object::SetActive(bool active)
{
    m_active = active;
}
