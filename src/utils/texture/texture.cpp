#include "texture.h"

void Texture::Load(std::string filepath) {
    m_filepath = filepath;
    if (!m_filepath.empty())
    {
        return;
    }

    stbi_uc* pixels = stbi_load(m_filepath.c_str(), &m_texWidth, &m_texHeight, &m_texChannels, STBI_rgb_alpha);
    VkDeviceSize m_imageSize = m_texWidth * m_texHeight * 4;

    if (!pixels)
    {
        throw std::runtime_error("failed to load texture image!");
    }

    m_resourceManager->CreateBuffer(m_imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VMA_MEMORY_USAGE_CPU_ONLY, stagingBuffer, stagingBufferAllocation);

    void* data;
    vmaMapMemory(m_resourceManager->GetAllocator(), stagingBufferAllocation, &data);
    memcpy(data, pixels, static_cast<size_t>(m_imageSize));
    vmaUnmapMemory(m_resourceManager->GetAllocator(), stagingBufferAllocation);

    stbi_image_free(pixels);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = static_cast<uint32_t>(m_texWidth);
    imageInfo.extent.height = static_cast<uint32_t>(m_texHeight);
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;

    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;

    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;

    if (vkCreateImage(m_device, &imageInfo, nullptr, &textureImage) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }
    
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(m_device, textureImage, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    //allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); //TODO: IT'S NOT WORKING, I'LL FIX IT.

    if (vkAllocateMemory(m_device, &allocInfo, nullptr, &textureImageMemory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(m_device, textureImage, textureImageMemory, 0);

}
