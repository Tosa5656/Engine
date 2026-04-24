#include "renderer/renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer()
{
    Destroy();
}

void Renderer::Init(GLFWwindow *window)
{
    m_window = window;

    VkApplicationInfo  appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Engine Editor";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_4;

    m_instance.Create(appInfo);
    m_instance.SetupDebugMessenger();
    m_surface.Create(&m_instance, m_window);
    m_device.PickPhysicalDevice(&m_instance, &m_surface);
    m_device.Create(&m_instance, &m_surface);
    m_resourceManager.Create(&m_device, &m_swapChain, &m_instance);
    m_resourceManager.CreateAllocator();
    m_swapChain.Create(&m_device, m_window, &m_surface);
    m_swapChain.CreateImageViews(&m_device);
    m_descriptorManager.Init(&m_device, &m_swapChain, &m_resourceManager);
    m_descriptorManager.CreateDescriptorSetLayout();
    m_pipelineManager.Create(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout());
    m_commandBufferManager.Init(m_device.GetDevice(), m_device.GetGraphicsQueueFamilyIndex(&m_surface));
    m_resourceManager.CreateUniformBuffers();

    std::vector<MeshVertex> vertices = {
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
        {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
        {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
        {{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}
    };
    std::vector<MeshIndex> indices = {
        {0}, {1}, {2}, {2}, {3}, {0}
    };
    m_mesh.Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), vertices, indices);
    m_mesh.LoadFromFile("models/cube.obj");

    m_descriptorManager.CreateDescriptorPool();
    m_descriptorManager.CreateDescriptorSets();
    CreateSyncObjects();
    m_imagesInFlight.resize(m_swapChain.GetSwapChainImages().size(), VK_NULL_HANDLE);
}

void Renderer::Draw()
{
    vkWaitForFences(m_device.GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_device.GetDevice(),
        m_swapChain.GetSwapChain(),
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_swapChain.Recreate(&m_device, m_window, &m_surface, &m_commandBufferManager);
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_device.GetDevice(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    m_resourceManager.UpdateUniformBuffer(imageIndex);

    VkCommandBuffer cmd = m_commandBufferManager.GetCommandBuffer(m_currentFrame);
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);

    VkImageMemoryBarrier barrier{};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.image = m_swapChain.GetSwapChainImages()[imageIndex];
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.srcAccessMask = 0;
    barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;

    vkCmdBeginRendering(cmd, &renderingInfo);

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGraphicsPipeline());

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChain.GetSwapChainExtent().width;
    viewport.height = (float)m_swapChain.GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain.GetSwapChainExtent();
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);

    m_mesh.Draw(cmd);

    vkCmdEndRendering(cmd);

    barrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    barrier.dstAccessMask = 0;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &barrier);

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS)
        throw std::runtime_error("failed to record command buffer!");

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[m_currentFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[imageIndex] };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    vkResetFences(m_device.GetDevice(), 1, &m_inFlightFences[m_currentFrame]);

    if (vkQueueSubmit(m_device.GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    VkSwapchainKHR swapChain = m_swapChain.GetSwapChain();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_device.GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        m_swapChain.Recreate(&m_device, m_window, &m_surface, &m_commandBufferManager);
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::Destroy()
{
    vkDeviceWaitIdle(m_device.GetDevice());

    for (auto& sem : m_imageAvailableSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
            vkDestroySemaphore(m_device.GetDevice(), sem, nullptr);
    }
    for (auto& sem : m_renderFinishedSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
            vkDestroySemaphore(m_device.GetDevice(), sem, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (m_inFlightFences[i] != VK_NULL_HANDLE)
            vkDestroyFence(m_device.GetDevice(), m_inFlightFences[i], nullptr);
    }

    m_swapChain.Cleanup(&m_device);
    m_pipelineManager.Shutdown(&m_device);
    m_descriptorManager.Cleanup();
    m_mesh.Destroy();
    m_commandBufferManager.Shutdown();
    m_resourceManager.Cleanup();

    vkDeviceWaitIdle(m_device.GetDevice());

    if (m_resourceManager.GetAllocator() != VK_NULL_HANDLE)
    {
        vmaDestroyAllocator(m_resourceManager.GetAllocator());
    }

    vkDestroyDevice(m_device.GetDevice(), nullptr);

    if (m_instance.IsExtensionValidationEnabled())
        m_instance.DestroyDebugUtilsMessengerEXT(m_instance.GetInstance(), m_instance.GetDebugMessenger(), nullptr);

    m_surface.Cleanup(&m_instance);
    vkDestroyInstance(m_instance.GetInstance(), nullptr);
}

void Renderer::SetFramebufferResized(bool resized)
{
    m_framebufferResized = resized;
}

VkDevice Renderer::GetDevice()
{
    return m_device.GetDevice();
}

void Renderer::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    uint32_t imageCount = static_cast<uint32_t>(m_swapChain.GetSwapChainImages().size());

    m_renderFinishedSemaphores.resize(imageCount);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (vkCreateSemaphore(m_device.GetDevice(), &semInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        if (vkCreateSemaphore(m_device.GetDevice(), &semInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }
}

void Renderer::CreatePerImageSemaphores()
{
    uint32_t imageCount = static_cast<uint32_t>(m_swapChain.GetSwapChainImages().size());

    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for (uint32_t i = 0; i < imageCount; i++)
    {
        if (vkCreateSemaphore(m_device.GetDevice(), &semInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device.GetDevice(), &semInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphore for swapchain image!");
        }
    }
}