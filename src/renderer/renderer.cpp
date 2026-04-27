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
    m_descriptorManager.Init(&m_device, &m_swapChain, &m_resourceManager, 256);
    m_descriptorManager.CreateDescriptorSetLayout();
    m_pipelineManager.Create(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1));
    m_commandBufferManager.Init(m_device.GetDevice(), m_device.GetGraphicsQueueFamilyIndex(&m_surface));
    m_resourceManager.CreateUniformBuffers();
    m_resourceManager.CreateObjectBuffer(256);

    m_objects.resize(10);
    m_materials.resize(10);
    m_materials[0] = Material(glm::vec3(1.0f, 0.0f, 0.0f), 0.0f, 0.3f);
    m_materials[1] = Material(glm::vec3(0.0f, 1.0f, 0.0f), 1.0f, 0.5f);
    m_materials[2] = Material(glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 0.7f);
    m_materials[3] = Material(glm::vec3(1.0f, 1.0f, 0.0f), 0.5f, 0.3f);
    m_materials[4] = Material(glm::vec3(1.0f, 0.0f, 1.0f), 0.5f, 0.5f);
    m_materials[5] = Material(glm::vec3(0.0f, 1.0f, 1.0f), 0.5f, 0.7f);
    m_materials[6] = Material(glm::vec3(1.0f, 1.0f, 1.0f), 0.8f, 0.2f);
    m_materials[7] = Material(glm::vec3(0.5f, 0.3f, 0.1f), 0.0f, 0.6f);
    m_materials[8] = Material(glm::vec3(0.3f, 0.5f, 0.8f), 0.9f, 0.1f);
    m_materials[9] = Material(glm::vec3(0.8f, 0.2f, 0.4f), 0.3f, 0.4f);

    for (size_t i = 0; i < m_objects.size(); i++)
    {
        m_objects[i].Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cat.obj");
        m_objects[i].SetMaterial(&m_materials[i]);
        float angle = (float)i * 6.28f / 10.0f;
        m_objects[i].GetTransform()->SetPosition(glm::vec3(glm::cos(angle) * 300.0f, 0.0f, glm::sin(angle) * 300.0f));
    }

    m_camera.SetPosition(glm::vec3(800.0f, 500.0f, 800.0f));
    m_camera.SetAspectRatio(m_swapChain.GetSwapChainExtent().width / (float)m_swapChain.GetSwapChainExtent().height);

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

    m_orbitYaw += 0.001f;
    m_resourceManager.UpdatePerFrameUBO(imageIndex, m_camera, glm::vec3(0.0f, 100.0f, 0.0f), m_orbitDistance, m_orbitYaw, m_orbitPitch);

    for (auto& obj : m_objects)
    {
        obj.UpdateUBO(&m_resourceManager);
    }

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

    for (auto& obj : m_objects)
    {
        uint32_t dynamicOffset = obj.GetUBOSlot() * m_resourceManager.GetObjectUBOStride();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);
        obj.Draw(cmd, m_descriptorManager.GetPerObjectDescriptorSets()[0], m_resourceManager.GetObjectUBOStride());
    }

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

    for (auto& obj : m_objects)
    {
        obj.Destroy();
    }

    m_commandBufferManager.Shutdown();
    m_resourceManager.Cleanup();

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