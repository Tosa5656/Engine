#include "renderer/renderer.h"

Renderer::Renderer() {}

Renderer::~Renderer()
{
    Destroy();
}

void Renderer::Init(GLFWwindow *window, Input* input)
{
    m_window = window;
    m_input = input;

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
    m_swapChain.Create(&m_device, m_window, &m_surface, m_resourceManager.GetAllocator());
    m_swapChain.CreateImageViews(&m_device);
    m_descriptorManager.Init(&m_device, &m_swapChain, &m_resourceManager, 256);
    m_descriptorManager.CreateDescriptorSetLayout();
    m_pipelineManager.Create(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout());
    m_commandBufferManager.Init(m_device.GetDevice(), m_device.GetGraphicsQueueFamilyIndex(&m_surface));
    m_resourceManager.CreateUniformBuffers();
    m_resourceManager.CreateObjectBuffer(4);

    m_material.SetAlbedo(glm::vec3(1.0f, 0.0f, 0.0f));
    m_material2.SetAlbedo(glm::vec3(0.0f, 1.0f, 0.0f));
    m_material3.SetAlbedo(glm::vec3(0.0f, 0.0f, 0.0f));

    m_material.Init(&m_device, m_resourceManager.GetAllocator());
    m_texture.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_texture.Load("textures/wall.jpg");
    m_material.SetTexture(&m_texture);

    m_material2.Init(&m_device, m_resourceManager.GetAllocator());
    m_texture2.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_texture2.Load("textures/container.jpg");
    m_material2.SetTexture(&m_texture2);
    m_material3.Init(&m_device, m_resourceManager.GetAllocator());
    m_texture3.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_texture3.Load("textures/awesomeface.png");
    m_material3.SetTexture(&m_texture3);

    m_scene.Init();

    Object* obj1 = new Object();
    obj1->Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj1->SetMaterial(&m_material);
    obj1->GetTransform()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    m_scene.AddObject(obj1);

    Object* obj2 = new Object();
    obj2->Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj2->SetMaterial(&m_material2);
    obj2->GetTransform()->SetPosition(glm::vec3(3.0f, 0.0f, 0.0f));
    m_scene.AddObject(obj2);

    Object* obj3 = new Object();
    obj3->Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj3->SetMaterial(&m_material3);
    obj3->GetTransform()->SetPosition(glm::vec3(-3.0f, 0.0f, 0.0f));
    m_scene.AddObject(obj3);

    m_scene.GetCamera()->SetPosition(glm::vec3(8.0f, 5.0f, 8.0f));
    m_scene.GetCamera()->SetAspectRatio(m_swapChain.GetSwapChainExtent().width / (float)m_swapChain.GetSwapChainExtent().height);

    m_descriptorManager.CreateDescriptorPool();
    m_descriptorManager.CreateDescriptorSets();

    if (m_material.HasTexture())
    {
        m_material.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material.GetTexture()));
    }

    if (m_material2.HasTexture())
    {
        m_material2.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material2.GetTexture()));
    }

    if (m_material3.HasTexture())
    {
        m_material3.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material3.GetTexture()));
    }

    m_resourceManager.CreateComputeResultBuffer();
    m_descriptorManager.CreateComputeDescriptorSetLayout();
    m_computePipeline.Create(&m_device, "shaders/compute.spv", m_descriptorManager.GetComputeDescriptorSetLayout());
    m_descriptorManager.CreateComputeDescriptorSet();

    CreateSyncObjects();
    m_imagesInFlight.resize(m_swapChain.GetSwapChainImages().size(), VK_NULL_HANDLE);

    m_device.CreateTimestampQueryPool();
}

void Renderer::Render()
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
        m_swapChain.Recreate(&m_device, m_window, &m_surface, &m_commandBufferManager, m_resourceManager.GetAllocator());
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

    m_resourceManager.UpdatePerFrameUBO(imageIndex, *m_scene.GetCamera());

    for (Object* obj : m_scene.GetObjects())
    {
        if (obj && obj->IsActive())
        {
            obj->UpdateUBO(&m_resourceManager);
        }
    }

    VkCommandBuffer cmd = m_commandBufferManager.GetCommandBuffer(m_currentFrame);
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);

    vkCmdResetQueryPool(cmd, m_device.GetTimestampQueryPool(), 0, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_device.GetTimestampQueryPool(), 0);

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

    VkImageMemoryBarrier depthBarrier{};
    depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    depthBarrier.image = m_swapChain.GetDepthImage();
    depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthBarrier.subresourceRange.baseMipLevel = 0;
    depthBarrier.subresourceRange.levelCount = 1;
    depthBarrier.subresourceRange.baseArrayLayer = 0;
    depthBarrier.subresourceRange.layerCount = 1;
    depthBarrier.srcAccessMask = 0;
    depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;

    vkCmdPipelineBarrier(cmd,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        0,
        0, nullptr,
        0, nullptr,
        1, &depthBarrier);

    VkRenderingAttachmentInfo colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue = {{0.0f, 0.0f, 0.0f, 1.0f}};

    VkRenderingAttachmentInfo depthAttachment{};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.imageView = m_swapChain.GetDepthImageView();
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.clearValue.depthStencil = {1.0f, 0};

    VkRenderingInfo renderingInfo{};
    renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderingInfo.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
    renderingInfo.layerCount = 1;
    renderingInfo.colorAttachmentCount = 1;
    renderingInfo.pColorAttachments = &colorAttachment;
    renderingInfo.pDepthAttachment = &depthAttachment;

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

    for (Object* obj : m_scene.GetObjects())
    {
        if (obj && obj->IsActive())
        {
            uint32_t dynamicOffset = obj->GetUBOSlot() * m_resourceManager.GetObjectUBOStride();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);

            Material* material = obj->GetMaterial();
            VkDescriptorSet textureDescriptorSet;
            if (material && material->HasTexture() && material->GetDescriptorSet() != VK_NULL_HANDLE)
            {
                textureDescriptorSet = material->GetDescriptorSet();
            }
            else
            {
                textureDescriptorSet = m_descriptorManager.GetNullTextureDescriptorSet();
            }
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 2, 1, &textureDescriptorSet, 0, nullptr);

            obj->Draw(cmd, m_descriptorManager.GetPerObjectDescriptorSets()[0], m_resourceManager.GetObjectUBOStride());
        }
    }

    vkCmdEndRendering(cmd);

    VkDescriptorSet computeDescriptorSet = m_descriptorManager.GetComputeDescriptorSet();
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline.GetPipeline());
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_computePipeline.GetPipelineLayout(), 0, 1, &computeDescriptorSet, 0, nullptr);
    vkCmdDispatch(cmd, 1, 1, 1);

    VkMemoryBarrier memoryBarrier{};
    memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    memoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
    memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = sizeof(float);
    vkCmdCopyBuffer(cmd, m_resourceManager.GetComputeResultBuffer(), m_resourceManager.GetComputeStagingBuffer(), 1, &copyRegion);

    VkMemoryBarrier readBarrier{};
    readBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
    readBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    readBarrier.dstAccessMask = VK_ACCESS_HOST_READ_BIT;
    vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, 0, 1, &readBarrier, 0, nullptr, 0, nullptr);

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

    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_device.GetTimestampQueryPool(), 1);

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

    vkWaitForFences(m_device.GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint64_t timestamps[2];
    vkGetQueryPoolResults(
        m_device.GetDevice(),
        m_device.GetTimestampQueryPool(),
        0,
        2,
        sizeof(timestamps),
        timestamps,
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    uint64_t deltaTicks = timestamps[1] - timestamps[0];
    m_deltaTime = static_cast<float>(deltaTicks * m_device.GetTimestampPeriod()) / 1e9f;

    if (!m_computeResultPrinted)
    {
        float result = 0.0f;
        m_resourceManager.ReadComputeResult(result);

        std::cout << "Compute result (Pi): " << result << std::endl;
        m_computeResultPrinted = true;
    }

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
        m_swapChain.Recreate(&m_device, m_window, &m_surface, &m_commandBufferManager, m_resourceManager.GetAllocator());
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    float moveSpeed = 300.0f * m_deltaTime;
    Camera* camera = m_scene.GetCamera();
    glm::vec3 forward = glm::normalize(camera->GetTarget() - camera->GetPosition());
    glm::vec3 right = glm::normalize(glm::cross(forward, camera->GetUp()));

    if (m_input->IsPressed(KeyCode::W))
        camera->Move(forward * moveSpeed);
    if (m_input->IsPressed(KeyCode::S))
        camera->Move(-forward * moveSpeed);
    if (m_input->IsPressed(KeyCode::A))
        camera->Move(-right * moveSpeed);
    if (m_input->IsPressed(KeyCode::D))
        camera->Move(right * moveSpeed);

    if (m_input->IsCursorCaptured())
    {
        glm::vec2 mouseDelta = m_input->GetMouseDelta();
        float rotateSpeed = 30 * m_deltaTime;
        camera->Rotate(-mouseDelta.x * rotateSpeed, -mouseDelta.y * rotateSpeed);
    }

    if (m_input->IsDown(KeyCode::G))
    {
        for (Object* obj : m_scene.GetObjects())
        {
            if (obj)
                obj->SetActive(!obj->IsActive());
        }
    }

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::Destroy()
{
    static bool alreadyDestroyed = false;
    if (alreadyDestroyed)
        return;
    alreadyDestroyed = true;

    VkDevice device = m_device.GetDevice();
    VkInstance instance = m_instance.GetInstance();

    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);

    for (auto& sem : m_imageAvailableSemaphores)
    {
        if (device != VK_NULL_HANDLE && sem != VK_NULL_HANDLE)
            vkDestroySemaphore(device, sem, nullptr);
    }
    for (auto& sem : m_renderFinishedSemaphores)
    {
        if (device != VK_NULL_HANDLE && sem != VK_NULL_HANDLE)
            vkDestroySemaphore(device, sem, nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
    {
        if (device != VK_NULL_HANDLE && m_inFlightFences[i] != VK_NULL_HANDLE)
            vkDestroyFence(device, m_inFlightFences[i], nullptr);
    }

    for (Object* obj : m_scene.GetObjects())
    {
        if (obj)
        {
            obj->Destroy();
            delete obj;
        }
    }
    m_scene.Destroy();
    m_mesh.Destroy();

    m_commandBufferManager.Shutdown();

    m_texture.Cleanup();
    m_texture2.Cleanup();
    m_texture3.Cleanup();

    m_resourceManager.Cleanup();

    m_computePipeline.Shutdown(&m_device);
    m_descriptorManager.Cleanup();
    m_pipelineManager.Shutdown(&m_device);
    m_swapChain.Cleanup(&m_device);

    m_device.DestroyTimestampQueryPool();

    VmaAllocator allocator = m_resourceManager.GetAllocator();
    if (allocator != VK_NULL_HANDLE)
        vmaDestroyAllocator(allocator);

    if (device != VK_NULL_HANDLE)
        vkDestroyDevice(device, nullptr);

    if (instance != VK_NULL_HANDLE)
    {
        if (m_instance.IsExtensionValidationEnabled())
            m_instance.DestroyDebugUtilsMessengerEXT(instance, m_instance.GetDebugMessenger(), nullptr);

        m_surface.Cleanup(&m_instance);
        vkDestroyInstance(instance, nullptr);
    }
}

void Renderer::SetFramebufferResized(bool resized)
{
    m_framebufferResized = resized;
}

VkDevice Renderer::GetDevice()
{
    return m_device.GetDevice();
}

float Renderer::GetDeltaTime()
{
    return m_deltaTime;
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