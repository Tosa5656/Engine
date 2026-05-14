#pragma once

#include <vector>
#include <stdexcept>

#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>

#include <renderer/vulkan/device.h>
#include <renderer/vulkan/commandbuffer.h>
#include <renderer/vulkan/surface.h>

class SwapChain
{
public:
    SwapChain();
    ~SwapChain();

    void Create(Device* device, GLFWwindow* window, Surface* surface, VmaAllocator allocator);
    void Recreate(Device* device, GLFWwindow* window, Surface* surface, CommandBufferManager* cmdManager, VmaAllocator allocator);
    void Cleanup(Device* device);

    void CreateDepthResources(Device* device, VmaAllocator allocator);
    void CreateImageViews(Device* device);
    void CreateGBufferResources(Device* device, VmaAllocator allocator);

    VkSwapchainKHR GetSwapChain();
    const std::vector<VkImage>& GetSwapChainImages() const { return m_swapChainImages; }
    VkFormat GetSwapChainImageFormat();
    VkExtent2D GetSwapChainExtent();
    const std::vector<VkImageView>& GetSwapChainImageViews() const { return m_swapChainImageViews; }
    VkImageView GetDepthImageView();
    VkFormat GetDepthFormat();
    VkImage GetDepthImage();
    VkImageView GetPositionImageView() const { return m_gPositionImageView; }
    VkImageView GetNormalImageView() const { return m_gNormalImageView; }
    VkImageView GetAlbedoImageView() const { return m_gAlbedoImageView; }
    VkImageView GetMaterialImageView() const { return m_gMaterialImageView; }
    VkImageView GetEmissiveAccumImageView() const { return m_emissiveAccumImageView; }
    VkImage GetPositionImage() const { return m_gPositionImage; }
    VkImage GetNormalImage() const { return m_gNormalImage; }
    VkImage GetAlbedoImage() const { return m_gAlbedoImage; }
    VkImage GetMaterialImage() const { return m_gMaterialImage; }
    VkImage GetEmissiveAccumImage() const { return m_emissiveAccumImage; }
    VkFormat GetPositionFormat() const { return VK_FORMAT_R32G32B32A32_SFLOAT; }
    VkFormat GetNormalFormat() const { return VK_FORMAT_R16G16B16A16_SFLOAT; }
    VkFormat GetAlbedoFormat() const { return VK_FORMAT_R8G8B8A8_SRGB; }
    VkFormat GetMaterialFormat() const { return VK_FORMAT_R8G8B8A8_UNORM; }
    VkFormat GetEmissiveAccumFormat() const { return VK_FORMAT_R16G16B16A16_SFLOAT; }
    VkImageView GetLightingResultImageView() const { return m_lightingResultImageView; }
    VkImage GetLightingResultImage() const { return m_lightingResultImage; }
    VkFormat GetLightingResultFormat() const { return VK_FORMAT_R16G16B16A16_SFLOAT; }

    VkImageView GetHdrColorImageView() const { return m_hdrColorImageView; }
    VkImage GetHdrColorImage() const { return m_hdrColorImage; }
    VkFormat GetHdrColorFormat() const { return VK_FORMAT_R16G16B16A16_SFLOAT; }

private:
    Device* m_device = nullptr;
    VmaAllocator m_allocator = VK_NULL_HANDLE;

    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    Surface* m_surface;
    std::vector<VkImage> m_swapChainImages;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    std::vector<VkImageView> m_swapChainImageViews;

    VkImage m_depthImage = VK_NULL_HANDLE;
    VmaAllocation m_depthImageAllocation = VK_NULL_HANDLE;
    VkImageView m_depthImageView = VK_NULL_HANDLE;
    VkFormat m_depthFormat = VK_FORMAT_D32_SFLOAT;

    VkImage m_gPositionImage = VK_NULL_HANDLE;
    VmaAllocation m_gPositionAllocation = VK_NULL_HANDLE;
    VkImageView m_gPositionImageView = VK_NULL_HANDLE;

    VkImage m_gNormalImage = VK_NULL_HANDLE;
    VmaAllocation m_gNormalAllocation = VK_NULL_HANDLE;
    VkImageView m_gNormalImageView = VK_NULL_HANDLE;

    VkImage m_gAlbedoImage = VK_NULL_HANDLE;
    VmaAllocation m_gAlbedoAllocation = VK_NULL_HANDLE;
    VkImageView m_gAlbedoImageView = VK_NULL_HANDLE;

    VkImage m_gMaterialImage = VK_NULL_HANDLE;
    VmaAllocation m_gMaterialAllocation = VK_NULL_HANDLE;
    VkImageView m_gMaterialImageView = VK_NULL_HANDLE;

    VkImage m_emissiveAccumImage = VK_NULL_HANDLE;
    VmaAllocation m_emissiveAccumAllocation = VK_NULL_HANDLE;
    VkImageView m_emissiveAccumImageView = VK_NULL_HANDLE;

    VkImage m_lightingResultImage = VK_NULL_HANDLE;
    VmaAllocation m_lightingResultAllocation = VK_NULL_HANDLE;
    VkImageView m_lightingResultImageView = VK_NULL_HANDLE;

    VkImage m_hdrColorImage = VK_NULL_HANDLE;
    VmaAllocation m_hdrColorAllocation = VK_NULL_HANDLE;
    VkImageView m_hdrColorImageView = VK_NULL_HANDLE;
};