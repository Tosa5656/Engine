#pragma once

#include <string>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <stb_image.h>

class Device;

class Texture
{
public:
    Texture();
    ~Texture();

    void Init(Device* device, VmaAllocator allocator, VkCommandPool commandPool);
    void Load(const std::string& path);
    void Cleanup();

    VkImageView GetImageView() const { return m_imageView; }
    VkSampler GetSampler() const { return m_sampler; }
    uint32_t GetWidth() const { return m_width; }
    uint32_t GetHeight() const { return m_height; }
    uint32_t GetMipLevels() const { return m_mipLevels; }

private:
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;

    VkImage m_image = VK_NULL_HANDLE;
    VmaAllocation m_allocation = VK_NULL_HANDLE;
    VkImageView m_imageView = VK_NULL_HANDLE;
    VkSampler m_sampler = VK_NULL_HANDLE;

    uint32_t m_width = 0;
    uint32_t m_height = 0;
    uint32_t m_mipLevels = 1;

    void CreateImage(uint32_t w, uint32_t h, uint32_t mips, VkFormat format, const void* data);
    void GenerateMipmaps(VkFormat format, int32_t w, int32_t h);
    void CreateImageView();
    void CreateSampler();
    void TransitionImageLayout(VkImage image, VkFormat format, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
};