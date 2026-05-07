#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/resources.h>
#include <renderer/vulkan/texture.h>

class Device;

class Material
{
public:
    Material();
    Material(glm::vec3 albedo, float metallic, float roughness, float ao = 1.0f, float normalStrength = 1.0f);
    Material(const Material&) = delete;
    Material& operator=(const Material&) = delete;
    ~Material();

    void SetAlbedo(glm::vec3 albedo);
    void SetMetallic(float metallic);
    void SetRoughness(float roughness);
    void SetAO(float ao);
    void SetNormalStrength(float normalStrength);
    void SetTexture(Texture* texture);
    void Init(Device* device, VmaAllocator allocator);

    glm::vec3 GetAlbedo() const;
    float GetMetallic() const;
    float GetRoughness() const;
    float GetAO() const;
    float GetNormalStrength() const;
    Texture* GetTexture() const { return m_texture; }
    bool HasTexture() const { return m_texture != nullptr; }

    const PerObjectUBO& GetData() const;

    void SetDescriptorSet(VkDescriptorSet descriptorSet) { m_textureDescriptorSet = descriptorSet; }
    VkDescriptorSet GetDescriptorSet() const { return m_textureDescriptorSet; }

private:
    PerObjectUBO m_data;
    Texture* m_texture = nullptr;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
};