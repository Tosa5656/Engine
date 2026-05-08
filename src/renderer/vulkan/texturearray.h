#pragma once

#include <string>
#include <vector>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

#include <stb_image.h>

class Device;

struct TextureInfo
{
    uint32_t index = 0;
    glm::vec2 offset = glm::vec2(0.0f);
    glm::vec2 scale = glm::vec2(1.0f);
};

class TextureArray
{
public:
    TextureArray();
    ~TextureArray();

    void Init(Device* device, VmaAllocator allocator, VkCommandPool commandPool, uint32_t maxTextures = 32);
    void AddTexture(const std::string& path);
    void Build();
    void Cleanup();

    VkImageView GetImageView() const { return m_imageView; }
    VkSampler GetSampler() const { return m_sampler; }
    uint32_t GetTextureCount() const { return static_cast<uint32_t>(m_textures.size()); }
    const TextureInfo& GetTextureInfo(uint32_t index) const { return m_textures[index]; }

private:
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    struct LoadedTexture
    {
        std::vector<stbi_uc*> pixels;
        uint32_t width = 0;
        uint32_t height = 0;
        uint32_t mipLevels = 1;
    };
    std::vector<LoadedTexture> m_loadedTextures;
    std::vector<TextureInfo> m_textures;

    VkImage m_image = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    uint32_t m_maxWidth = 0;
    uint32_t m_maxHeight = 0;
    uint32_t m_maxTextures = 32;
    bool m_built = false;

    void CreateAtlasImage();
    void CreateImageView();
    void CreateSampler();
    void TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t offsetX, uint32_t offsetY);
    uint32_t GetMipLevelCount(uint32_t width, uint32_t height);
};