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
        VkDeviceMemory textureImageMemory;
        VkBuffer stagingBuffer;
        VmaAllocation stagingBufferAllocation;

        VkDevice m_device = m_resourceManager->GetVkDevice();

    public:
        Texture();

        ~Texture()
        {
            DestroyImage();
        };

        void Load(std::string filepath);
        void DestroyImage();
};
