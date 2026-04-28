#pragma once
#include <string>
#include <vulkan/vulkan.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stdexcept>
#include "renderer/vulkan/resources.h"

class Texture
{
    private:
        std::string m_filepath = "";
        int m_texWidth, m_texHeight, m_texChannels = 0;

        ResourceManager* m_resourceManager;

        VkImage textureImage;
        VmaAllocation textureImageMemory;
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;
        VmaAllocator m_allocator;

        VkDevice m_device;

    public:
        Texture();

        ~Texture()
        {
            DestroyImage();
        };

        void Load(std::string filepath);
        void DestroyImage();
};
