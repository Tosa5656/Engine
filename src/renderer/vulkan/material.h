#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <renderer/vulkan/resources.h>
#include <renderer/vulkan/texture.h>
#include <renderer/vulkan/texturearray.h>
#include <cstdint>

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
    void SetTextureArray(TextureArray* textureArray, uint32_t textureIndex);
    void SetNormalMap(Texture* normalMap);
    void SetHeightMap(Texture* heightMap);
    void SetParallaxMode(ParallaxMode mode);
    void SetParallaxScale(float scale);
    void SetParallaxIterations(int iterations);
    void Init(Device* device, VmaAllocator allocator);

    glm::vec3 GetAlbedo() const;
    float GetMetallic() const;
    float GetRoughness() const;
    float GetAO() const;
    float GetNormalStrength() const;
    ParallaxMode GetParallaxMode() const { return m_parallaxMode; }
    float GetParallaxScale() const { return m_parallaxScale; }
    int GetParallaxIterations() const { return m_parallaxIterations; }
    Texture* GetTexture() const { return m_texture; }
    TextureArray* GetTextureArray() const { return m_textureArray; }
    Texture* GetNormalMap() const { return m_normalMap; }
    Texture* GetHeightMap() const { return m_heightMap; }
    bool HasTexture() const { return m_texture != nullptr || m_textureArray != nullptr; }
    uint32_t GetTextureIndex() const { return m_textureIndex; }

    const PerObjectUBO& GetData() const;

    void SetDescriptorSet(VkDescriptorSet descriptorSet) { m_textureDescriptorSet = descriptorSet; }
    VkDescriptorSet GetDescriptorSet() const { return m_textureDescriptorSet; }
    void SetNormalMapDescriptorSet(VkDescriptorSet descriptorSet) { m_normalMapDescriptorSet = descriptorSet; }
    VkDescriptorSet GetNormalMapDescriptorSet() const { return m_normalMapDescriptorSet; }
    void SetHeightMapDescriptorSet(VkDescriptorSet descriptorSet) { m_heightMapDescriptorSet = descriptorSet; }
    VkDescriptorSet GetHeightMapDescriptorSet() const { return m_heightMapDescriptorSet; }

private:
    PerObjectUBO m_data;
    Texture* m_texture = nullptr;
    TextureArray* m_textureArray = nullptr;
    uint32_t m_textureIndex = 0;
    Texture* m_normalMap = nullptr;
    Texture* m_heightMap = nullptr;
    VkDescriptorSet m_textureDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet m_normalMapDescriptorSet = VK_NULL_HANDLE;
    VkDescriptorSet m_heightMapDescriptorSet = VK_NULL_HANDLE;
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    ParallaxMode m_parallaxMode = ParallaxMode::ReliefMapping;
    float m_parallaxScale = 0.1f;
    int m_parallaxIterations = 32;
};