#include "texturearray.h"

#include <cmath>
#include <memory>
#include <stdexcept>

#include <renderer/vulkan/device.h>

TextureArray::TextureArray() = default;

TextureArray::~TextureArray()
{
    Cleanup();
}

void TextureArray::Init(Device* device, VmaAllocator allocator, VkCommandPool commandPool, uint32_t maxTextures)
{
    m_device = device;
    m_allocator = allocator;
    m_commandPool = commandPool;
    m_maxTextures = maxTextures;
}

void TextureArray::AddTexture(const std::string& path)
{
    if (m_built)
        throw std::runtime_error("Cannot add texture after atlas is built!");

    if (m_loadedTextures.size() >= m_maxTextures)
        throw std::runtime_error("Maximum texture count reached!");

    int texWidth, texHeight, texChannels;
    auto pixels = std::unique_ptr<stbi_uc, decltype(&stbi_image_free)>(
        stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha),
        stbi_image_free);

    if (!pixels)
        throw std::runtime_error("failed to load texture: " + path);

    LoadedTexture loaded;
    loaded.width = static_cast<uint32_t>(texWidth);
    loaded.height = static_cast<uint32_t>(texHeight);
    loaded.mipLevels = GetMipLevelCount(loaded.width, loaded.height);

    m_maxWidth = std::max(m_maxWidth, loaded.width);
    m_maxHeight = std::max(m_maxHeight, loaded.height);

    TextureInfo info;
    info.index = static_cast<uint32_t>(m_loadedTextures.size());
    m_textures.push_back(info);

    loaded.pixels.push_back(pixels.get());
    pixels.release();
    m_loadedTextures.push_back(std::move(loaded));
}

void TextureArray::Build()
{
    if (m_built)
        return;

    if (m_loadedTextures.empty())
        throw std::runtime_error("No textures to build atlas!");

    CreateAtlasImage();
    m_built = true;
}

uint32_t TextureArray::GetMipLevelCount(uint32_t width, uint32_t height)
{
    return static_cast<uint32_t>(std::floor(std::log2(std::max(width, height))) + 1);
}

void TextureArray::CreateAtlasImage()
{
    uint32_t maxMip = GetMipLevelCount(m_maxWidth, m_maxHeight);

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.extent.width = m_maxWidth;
    imageInfo.extent.height = m_maxHeight * static_cast<uint32_t>(m_loadedTextures.size());
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo allocInfo{};
    allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if (vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &m_image, &m_allocation, nullptr) != VK_SUCCESS)
        throw std::runtime_error("failed to create atlas image!");

    TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    for (size_t i = 0; i < m_loadedTextures.size(); i++)
    {
        LoadedTexture& tex = m_loadedTextures[i];

        VkDeviceSize imageSize = static_cast<VkDeviceSize>(tex.width * tex.height * 4);

        VkBuffer stagingBuffer;
        VmaAllocation stagingAllocation;

        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = imageSize;
        bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo stagingAllocInfo{};
        stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

        if (vmaCreateBuffer(m_allocator, &bufferInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to create staging buffer!");

        void* mappedData;
        vmaMapMemory(m_allocator, stagingAllocation, &mappedData);
        memcpy(mappedData, tex.pixels[0], static_cast<size_t>(imageSize));
        vmaUnmapMemory(m_allocator, stagingAllocation);

        uint32_t offsetX = 0;
        uint32_t offsetY = static_cast<uint32_t>(i) * m_maxHeight;
        uint32_t totalHeight = m_maxHeight * static_cast<uint32_t>(m_loadedTextures.size());

        CopyBufferToImage(stagingBuffer, m_image, tex.width, tex.height, offsetX, offsetY);

        m_textures[i].offset = glm::vec2(
            static_cast<float>(offsetX) / static_cast<float>(m_maxWidth),
            static_cast<float>(offsetY) / static_cast<float>(totalHeight)
        );
        m_textures[i].scale = glm::vec2(
            static_cast<float>(tex.width) / static_cast<float>(m_maxWidth),
            static_cast<float>(tex.height) / static_cast<float>(totalHeight)
        );

        vmaDestroyBuffer(m_allocator, stagingBuffer, stagingAllocation);

        stbi_image_free(tex.pixels[0]);
        tex.pixels.clear();
    }

    TransitionImageLayout(m_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    CreateImageView();
    CreateSampler();
}

void TextureArray::CreateImageView()
{
    uint32_t maxMip = GetMipLevelCount(m_maxWidth, m_maxHeight);

    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;

    if (vkCreateImageView(m_device->GetDevice(), &viewInfo, nullptr, &m_imageView) != VK_SUCCESS)
        throw std::runtime_error("failed to create atlas image view!");
}

void TextureArray::CreateSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.anisotropyEnable = VK_FALSE;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;

    if (vkCreateSampler(m_device->GetDevice(), &samplerInfo, nullptr, &m_sampler) != VK_SUCCESS)
        throw std::runtime_error("failed to create atlas sampler!");
}

void TextureArray::TransitionImageLayout(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffer!");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        throw std::runtime_error("unsupported layout transition!");
    }

    vkCmdPipelineBarrier(cmdBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->GetGraphicsQueue());

    vkFreeCommandBuffers(m_device->GetDevice(), m_commandPool, 1, &cmdBuffer);
}

void TextureArray::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t offsetX, uint32_t offsetY)
{
    VkCommandBuffer cmdBuffer;

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(m_device->GetDevice(), &allocInfo, &cmdBuffer) != VK_SUCCESS)
        throw std::runtime_error("failed to allocate command buffer!");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(cmdBuffer, &beginInfo);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {static_cast<int32_t>(offsetX), static_cast<int32_t>(offsetY), 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(cmdBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    vkEndCommandBuffer(cmdBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmdBuffer;

    vkQueueSubmit(m_device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(m_device->GetGraphicsQueue());

    vkFreeCommandBuffers(m_device->GetDevice(), m_commandPool, 1, &cmdBuffer);
}

void TextureArray::Cleanup()
{
    if (m_device == nullptr || m_device->GetDevice() == VK_NULL_HANDLE)
        return;

    VkDevice device = m_device->GetDevice();

    if (m_sampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, m_sampler, nullptr);
        m_sampler = VK_NULL_HANDLE;
    }

    if (m_imageView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, m_imageView, nullptr);
        m_imageView = VK_NULL_HANDLE;
    }

    if (m_image != VK_NULL_HANDLE && m_allocation != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_allocator, m_image, m_allocation);
        m_image = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
    }

    for (auto& tex : m_loadedTextures)
    {
        for (auto* pixel : tex.pixels)
        {
            if (pixel)
                stbi_image_free(pixel);
        }
        tex.pixels.clear();
    }
    m_loadedTextures.clear();
    m_textures.clear();
}