#pragma once
#include <string>
#include <vulkan/vulkan.h>
#include <stdexcept>
#include "renderer/vulkan/resources.h"
#include "renderer/vulkan/commandbuffer.h"

class Texture
{
    private:
        std::string m_filepath = "";
        int m_texWidth, m_texHeight, m_texChannels = 0;

        ResourceManager* m_resourceManager;
        CommandBufferManager* m_commandBufferManager;

        VkImage textureImage;
        VmaAllocation textureImageMemory;
        VkImageView textureImageView;
        VkSampler textureSampler;
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocator m_allocator;

        Device* m_device;

    public:
        Texture(ResourceManager* resourceManager, Device* device, CommandBufferManager* commandBufferManager);

        ~Texture()
        {
            DestroyImage();
        };

        void Load(std::string filepath);
        void DestroyImage();

        VkImageView GetImageView() { return textureImageView; }
        VkSampler GetSampler() { return textureSampler; }
};
