#pragma once

#include <vector>
#include <array>

#include <vulkan/vulkan.h>

#include <renderer/vulkan/device.h>

constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

class CommandBufferManager
{
public:
    CommandBufferManager();
    ~CommandBufferManager();

    void Init(VkDevice device, uint32_t graphicsQueueFamilyIndex);
    void Shutdown();
    void Recreate(uint32_t queueFamilyIndex);

    VkCommandBuffer ResetAndBegin(uint32_t frameIndex);
    void End(VkCommandBuffer cmd);

    VkCommandBuffer GetCommandBuffer(uint32_t frameIndex) const;
    VkCommandPool GetCommandPool() const;

    void ResetAll();

private:
    VkDevice m_device = VK_NULL_HANDLE;
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::array<VkCommandBuffer, MAX_FRAMES_IN_FLIGHT> m_primaryCommandBuffers{};

    bool m_initialized = false;
};