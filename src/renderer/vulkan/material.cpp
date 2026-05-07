#include "material.h"

#include <renderer/vulkan/device.h>

Material::Material()
{
    m_data.model = glm::mat4(1.0f);
    m_data.albedo = glm::vec3(0.5f, 0.5f, 0.5f);
    m_data.metallic = 0.0f;
    m_data.roughness = 0.5f;
    m_data.ao = 1.0f;
    m_data.normalStrength = 1.0f;
}

Material::Material(glm::vec3 albedo, float metallic, float roughness, float ao, float normalStrength)
{
    m_data.model = glm::mat4(1.0f);
    m_data.albedo = albedo;
    m_data.metallic = metallic;
    m_data.roughness = roughness;
    m_data.ao = ao;
    m_data.normalStrength = normalStrength;
}

Material::~Material() = default;

void Material::SetAlbedo(glm::vec3 albedo)
{
    m_data.albedo = albedo;
}

void Material::SetMetallic(float metallic)
{
    m_data.metallic = metallic;
}

void Material::SetRoughness(float roughness)
{
    m_data.roughness = roughness;
}

void Material::SetAO(float ao)
{
    m_data.ao = ao;
}

void Material::SetNormalStrength(float normalStrength)
{
    m_data.normalStrength = normalStrength;
}

glm::vec3 Material::GetAlbedo() const
{
    return m_data.albedo;
}

float Material::GetMetallic() const
{
    return m_data.metallic;
}

float Material::GetRoughness() const
{
    return m_data.roughness;
}

float Material::GetAO() const
{
    return m_data.ao;
}

float Material::GetNormalStrength() const
{
    return m_data.normalStrength;
}

const PerObjectUBO& Material::GetData() const
{
    return m_data;
}

void Material::SetTexture(Texture* texture)
{
    m_texture = texture;
    m_textureArray = nullptr;
}

void Material::SetTextureArray(TextureArray* textureArray, uint32_t textureIndex)
{
    m_textureArray = textureArray;
    m_textureIndex = textureIndex;
    m_texture = nullptr;
}

void Material::Init(Device* device, VmaAllocator allocator)
{
    m_device = device;
    m_allocator = allocator;
}