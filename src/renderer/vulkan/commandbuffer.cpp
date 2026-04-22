#include "commandbuffer.h"

CommandBufferManager::CommandBufferManager() {};
CommandBufferManager::~CommandBufferManager() {};

void CommandBufferManager::Init(VkDevice device, uint32_t graphicsQueueFamilyIndex)
{
    if (m_initialized) return;

    m_device = device;

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = graphicsQueueFamilyIndex;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to create command pool!");

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(device, &allocInfo, m_primaryCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to allocate command buffers!");

    m_initialized = true;
}

void CommandBufferManager::Shutdown()
{
    if (!m_initialized) return;

    if (m_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        m_commandPool = VK_NULL_HANDLE;
    }

    m_device = VK_NULL_HANDLE;
    m_initialized = false;
}

void CommandBufferManager::Recreate(uint32_t queueFamilyIndex)
{
    if (m_commandPool != VK_NULL_HANDLE)
    {
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
        throw std::runtime_error("Failed to recreate command pool!");

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_primaryCommandBuffers.data()) != VK_SUCCESS)
        throw std::runtime_error("Failed to recreate command buffers!");
}

VkCommandBuffer CommandBufferManager::ResetAndBegin(uint32_t frameIndex)
{
    VkCommandBuffer cmd = m_primaryCommandBuffers[frameIndex];

    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("Failed to begin recording command buffer!");

    return cmd;
}

void CommandBufferManager::End(VkCommandBuffer cmd)
{
    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error("Failed to end command buffer recording!");
}

VkCommandBuffer CommandBufferManager::GetCommandBuffer(uint32_t frameIndex) const
{
    return m_primaryCommandBuffers[frameIndex];
}

VkCommandPool CommandBufferManager::GetCommandPool() const
{
    return m_commandPool;
}

VkCommandBuffer CommandBufferManager::AllocateSecondary(uint32_t frameIndex)
{
    throw std::runtime_error("Secondary command buffers not implemented yet");
}

void CommandBufferManager::BeginSecondary(VkCommandBuffer cmd, VkFramebuffer framebuffer)
{

}