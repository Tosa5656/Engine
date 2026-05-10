#include "renderer/renderer.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <renderer/vulkan/light/directional.h>
#include <renderer/vulkan/light/point.h>
#include <renderer/vulkan/light/spot.h>
#include <renderer/vulkan/light/debug_mesh.h>

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
    m_pipelineManager.Create(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout(), m_descriptorManager.GetNormalMapSetLayout(), m_descriptorManager.GetHeightMapSetLayout(), m_descriptorManager.GetLightSetLayout());
    m_pipelineManager.CreateLinePipeline(&m_device, &m_swapChain);
    m_commandBufferManager.Init(m_device.GetDevice(), m_device.GetGraphicsQueueFamilyIndex(&m_surface));
    m_resourceManager.CreateUniformBuffers();
    m_resourceManager.CreateObjectBuffer(128);
    m_resourceManager.CreateLightBuffer(8);

#ifndef NDEBUG
    {
        std::vector<MeshVertex> sphereVerts;
        std::vector<MeshIndex> sphereIndices;
        GenerateSphereMesh(sphereVerts, sphereIndices, 1.0f, 16, 12);
        m_debugSphere.Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), sphereVerts, sphereIndices);

        std::vector<MeshVertex> coneVerts;
        std::vector<MeshIndex> coneIndices;
        GenerateConeMesh(coneVerts, coneIndices, 1.0f, 1.0f, 16);
        m_debugCone.Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), coneVerts, coneIndices);

        std::vector<MeshVertex> arrowVerts;
        std::vector<MeshIndex> arrowIndices;
        GenerateArrowMesh(arrowVerts, arrowIndices, 1.0f, 0.3f, 0.15f);
        m_debugArrow.Init(&m_device, &m_commandBufferManager, m_resourceManager.GetAllocator(), arrowVerts, arrowIndices);
    }
#endif

    m_material.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    m_material2.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    m_material3.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));

    m_material.Init(&m_device, m_resourceManager.GetAllocator());
    m_material2.Init(&m_device, m_resourceManager.GetAllocator());
    m_material3.Init(&m_device, m_resourceManager.GetAllocator());

    m_textureAtlas.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool(), 16);
    m_textureAtlas.AddTexture("textures/BrickWall23_1K_BaseColor.png");
    m_textureAtlas.Build();

    m_singleTexture.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_singleTexture.Load("textures/BrickWall23_1K_BaseColor.png");

    m_normalMap.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_normalMap.Load("textures/BrickWall23_1K_Normal.png");

    m_heightMap.Init(&m_device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_heightMap.Load("textures/BrickWall23_1K_Height.png");

    m_material.SetTextureArray(&m_textureAtlas, 0);
    m_material.SetNormalMap(&m_normalMap);
    m_material.SetHeightMap(&m_heightMap);
    m_material.SetParallaxMode(ParallaxMode::ReliefMapping);
    m_material.SetParallaxScale(0.05f);
    m_material.SetParallaxIterations(32);
    m_material2.SetTexture(&m_singleTexture);
    m_material3.SetTexture(nullptr);

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

    DirectionalLight* sun = new DirectionalLight();
    sun->SetDirection(glm::vec3(1.0f, -1.0f, 0.5f));
    sun->SetColor(glm::vec3(1.0f, 0.95f, 0.8f));
    sun->SetIntensity(10.5f);
    m_scene.AddLight(sun);

    m_descriptorManager.CreateDescriptorPool();
    m_descriptorManager.CreateDescriptorSets();

    if (m_material.HasTexture())
    {
        if (m_material.GetTextureArray())
            m_material.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material.GetTextureArray()));
        else if (m_material.GetTexture())
            m_material.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material.GetTexture()));
    }

    if (m_material2.HasTexture())
    {
        if (m_material2.GetTextureArray())
            m_material2.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material2.GetTextureArray()));
        else if (m_material2.GetTexture())
            m_material2.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material2.GetTexture()));
    }

    if (m_material3.HasTexture())
    {
        if (m_material3.GetTextureArray())
            m_material3.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material3.GetTextureArray()));
        else if (m_material3.GetTexture())
            m_material3.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material3.GetTexture()));
    }

    if (m_material.GetNormalMap())
    {
        m_material.SetNormalMapDescriptorSet(m_descriptorManager.CreateNormalMapDescriptorSet(m_material.GetNormalMap()));
    }

    if (m_material2.GetNormalMap())
    {
        m_material2.SetNormalMapDescriptorSet(m_descriptorManager.CreateNormalMapDescriptorSet(m_material2.GetNormalMap()));
    }

    if (m_material.GetHeightMap())
    {
        m_material.SetHeightMapDescriptorSet(m_descriptorManager.CreateHeightMapDescriptorSet(m_material.GetHeightMap()));
    }

    if (m_material2.GetHeightMap())
    {
        m_material2.SetHeightMapDescriptorSet(m_descriptorManager.CreateHeightMapDescriptorSet(m_material2.GetHeightMap()));
    }

    m_descriptorManager.CreateGBufferDescriptorSet();
    m_descriptorManager.CreateCompositeDescriptorSet();

    m_pipelineManager.CreateGBufferPipeline(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout(), m_descriptorManager.GetNormalMapSetLayout(), m_descriptorManager.GetHeightMapSetLayout());
    m_pipelineManager.CreateLightingPipeline(&m_device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetGBufferSetLayout(), m_descriptorManager.GetLightSetLayout());
    m_pipelineManager.CreateCompositePipeline(&m_device, &m_swapChain, m_descriptorManager.GetCompositeSetLayout());

    m_resourceManager.CreateComputeResultBuffer();
    m_descriptorManager.CreateComputeDescriptorSetLayout();
    m_computePipeline.Create(&m_device, "shaders/compute.spv", m_descriptorManager.GetComputeDescriptorSetLayout());
    m_descriptorManager.CreateComputeDescriptorSet();

    CreateSyncObjects();
    m_imagesInFlight.resize(m_swapChain.GetSwapChainImages().size(), VK_NULL_HANDLE);

    m_device.CreateTimestampQueryPool();

    m_gui.Init(m_device.GetDevice(), m_instance.GetInstance(), m_device.GetPhysicalDevice(), m_device.GetGraphicsQueueFamilyIndex(&m_surface), m_device.GetGraphicsQueue(), m_window, VK_NULL_HANDLE);
}

void Renderer::Render()
{
    m_gui.NewFrame();
    
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
        m_descriptorManager.UpdateGBufferDescriptorSet();
        m_descriptorManager.UpdateCompositeDescriptorSet();
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

    {
        auto& lights = m_scene.GetLights();
        if (!lights.empty() && lights[0] && lights[0]->GetType() == LightType::Directional)
        {
            static float angle = 0.0f;
            angle += m_deltaTime * 0.5f;
            DirectionalLight* sun = static_cast<DirectionalLight*>(lights[0]);
            glm::quat rot = glm::angleAxis(angle, glm::vec3(0.0f, 1.0f, 0.0f));
            sun->SetDirection(rot * glm::vec3(1.0f, -1.0f, 0.5f));
        }
    }
    m_scene.Update(m_deltaTime, &m_resourceManager);

    VkCommandBuffer cmd = m_commandBufferManager.GetCommandBuffer(m_currentFrame);
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    vkBeginCommandBuffer(cmd, &beginInfo);

    vkCmdResetQueryPool(cmd, m_device.GetTimestampQueryPool(), 0, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_device.GetTimestampQueryPool(), 0);

    auto transitionImage = [&](VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageAspectFlags aspectMask, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkPipelineStageFlags srcStage, VkPipelineStageFlags dstStage) {
        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspectMask;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;
        barrier.srcAccessMask = srcAccess;
        barrier.dstAccessMask = dstAccess;

        vkCmdPipelineBarrier(cmd, srcStage, dstStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);
    };

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)m_swapChain.GetSwapChainExtent().width;
    viewport.height = (float)m_swapChain.GetSwapChainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain.GetSwapChainExtent();

    {
        struct { VkImage img; VkFormat fmt; } gbufferRTs[4] = {
            { m_swapChain.GetPositionImage(),      m_swapChain.GetPositionFormat() },
            { m_swapChain.GetNormalImage(),        m_swapChain.GetNormalFormat() },
            { m_swapChain.GetAlbedoImage(),        m_swapChain.GetAlbedoFormat() },
            { m_swapChain.GetMaterialImage(),      m_swapChain.GetMaterialFormat() }
        };

        for (int i = 0; i < 4; i++)
            transitionImage(gbufferRTs[i].img, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        transitionImage(m_swapChain.GetDepthImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);

        VkRenderingAttachmentInfo gbufferAttachments[4]{};
        VkClearValue clearBlack = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
        for (int i = 0; i < 4; i++)
        {
            gbufferAttachments[i].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
            gbufferAttachments[i].imageView = i == 0 ? m_swapChain.GetPositionImageView() :
                                               i == 1 ? m_swapChain.GetNormalImageView() :
                                               i == 2 ? m_swapChain.GetAlbedoImageView() :
                                                        m_swapChain.GetMaterialImageView();
            gbufferAttachments[i].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
            gbufferAttachments[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            gbufferAttachments[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            gbufferAttachments[i].clearValue = clearBlack;
        }

        VkRenderingAttachmentInfo gbufferDepth{};
        gbufferDepth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        gbufferDepth.imageView = m_swapChain.GetDepthImageView();
        gbufferDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        gbufferDepth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        gbufferDepth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        gbufferDepth.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo gbufferRendering{};
        gbufferRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        gbufferRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        gbufferRendering.layerCount = 1;
        gbufferRendering.colorAttachmentCount = 4;
        gbufferRendering.pColorAttachments = gbufferAttachments;
        gbufferRendering.pDepthAttachment = &gbufferDepth;

        vkCmdBeginRendering(cmd, &gbufferRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipeline());
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);

        for (Object* obj : m_scene.GetObjects())
        {
            if (!obj || !obj->IsActive()) continue;

            uint32_t dynamicOffset = obj->GetUBOSlot() * m_resourceManager.GetObjectUBOStride();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);

            Material* material = obj->GetMaterial();
            VkDescriptorSet texDS = (material && material->HasTexture() && material->GetDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetDescriptorSet() : m_descriptorManager.GetNullTextureDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipelineLayout(), 2, 1, &texDS, 0, nullptr);

            VkDescriptorSet normDS = (material && material->GetNormalMapDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetNormalMapDescriptorSet() : m_descriptorManager.GetNullNormalMapDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipelineLayout(), 3, 1, &normDS, 0, nullptr);

            VkDescriptorSet heightDS = (material && material->GetHeightMapDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetHeightMapDescriptorSet() : m_descriptorManager.GetNullHeightMapDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetGBufferPipelineLayout(), 4, 1, &heightDS, 0, nullptr);

            obj->Draw(cmd, m_descriptorManager.GetPerObjectDescriptorSets()[0], m_resourceManager.GetObjectUBOStride());
        }

        vkCmdEndRendering(cmd);
    }

    {
        VkImage gbufferImages[5] = {
            m_swapChain.GetPositionImage(),
            m_swapChain.GetNormalImage(),
            m_swapChain.GetAlbedoImage(),
            m_swapChain.GetMaterialImage(),
            m_swapChain.GetDepthImage()
        };
        for (int i = 0; i < 4; i++)
            transitionImage(gbufferImages[i], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        transitionImage(gbufferImages[4], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        transitionImage(m_swapChain.GetLightingResultImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);

        transitionImage(m_swapChain.GetEmissiveAccumImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    {
        VkRenderingAttachmentInfo lightingAttachments[2]{};
        VkClearValue clearBlack = {{{0.0f, 0.0f, 0.0f, 0.0f}}};
        lightingAttachments[0].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        lightingAttachments[0].imageView = m_swapChain.GetLightingResultImageView();
        lightingAttachments[0].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        lightingAttachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        lightingAttachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        lightingAttachments[0].clearValue = clearBlack;

        lightingAttachments[1].sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        lightingAttachments[1].imageView = m_swapChain.GetEmissiveAccumImageView();
        lightingAttachments[1].imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        lightingAttachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        lightingAttachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        lightingAttachments[1].clearValue = clearBlack;

        VkRenderingInfo lightingRendering{};
        lightingRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        lightingRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        lightingRendering.layerCount = 1;
        lightingRendering.colorAttachmentCount = 2;
        lightingRendering.pColorAttachments = lightingAttachments;
        lightingRendering.pDepthAttachment = nullptr;

        vkCmdBeginRendering(cmd, &lightingRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLightingPipeline());
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLightingPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);
        VkDescriptorSet gbufferDS = m_descriptorManager.GetGBufferDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLightingPipelineLayout(), 1, 1, &gbufferDS, 0, nullptr);
        VkDescriptorSet lightDS = m_descriptorManager.GetLightDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLightingPipelineLayout(), 2, 1, &lightDS, 0, nullptr);

        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRendering(cmd);
    }

    {
        transitionImage(m_swapChain.GetLightingResultImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        transitionImage(m_swapChain.GetEmissiveAccumImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        transitionImage(m_swapChain.GetSwapChainImages()[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    {
        VkRenderingAttachmentInfo compositeAttachment{};
        compositeAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        compositeAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
        compositeAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        compositeAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        compositeAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo compositeRendering{};
        compositeRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        compositeRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        compositeRendering.layerCount = 1;
        compositeRendering.colorAttachmentCount = 1;
        compositeRendering.pColorAttachments = &compositeAttachment;
        compositeRendering.pDepthAttachment = nullptr;

        vkCmdBeginRendering(cmd, &compositeRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetCompositePipeline());
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkDescriptorSet compositeDS = m_descriptorManager.GetCompositeDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetCompositePipelineLayout(), 0, 1, &compositeDS, 0, nullptr);
        VkDescriptorSet emissiveDS = m_descriptorManager.GetEmissiveAccumDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetCompositePipelineLayout(), 1, 1, &emissiveDS, 0, nullptr);

        vkCmdDraw(cmd, 3, 1, 0, 0);
    }

#ifndef NDEBUG
    if (m_debugLightsEnabled)
    {
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLinePipeline());

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);
        VkDescriptorSet lightDS = m_descriptorManager.GetLightDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 5, 1, &lightDS, 0, nullptr);

        VkDescriptorSet nullTex = m_descriptorManager.GetNullTextureDescriptorSet();
        VkDescriptorSet nullNorm = m_descriptorManager.GetNullNormalMapDescriptorSet();
        VkDescriptorSet nullHeight = m_descriptorManager.GetNullHeightMapDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 2, 1, &nullTex, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 3, 1, &nullNorm, 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 4, 1, &nullHeight, 0, nullptr);

        std::vector<uint32_t> debugSlots;
        for (Light* light : m_scene.GetLights())
        {
            if (!light) continue;

            glm::mat4 model(1.0f);
            LightType type = light->GetType();
            glm::vec3 color = light->GetColor();

            if (type == LightType::Point)
            {
                const PointLight* pl = static_cast<const PointLight*>(light);
                glm::vec3 pos = pl->GetPosition();
                float radius = pl->GetRadius();
                model = glm::translate(model, pos);
                model = glm::scale(model, glm::vec3(radius));
            }
            else if (type == LightType::Spot)
            {
                const SpotLight* sl = static_cast<const SpotLight*>(light);
                glm::vec3 pos = sl->GetPosition();
                glm::vec3 dir = sl->GetDirection();
                float radius = sl->GetRadius();
                float height = radius;
                model = glm::translate(model, pos);
                glm::vec3 up(0.0f, 1.0f, 0.0f);
                glm::quat rot = glm::rotation(up, glm::normalize(dir));
                model = model * glm::mat4_cast(rot);
                model = glm::scale(model, glm::vec3(radius * tan(sl->GetOuterCutoff()), height, radius * tan(sl->GetOuterCutoff())));
            }
            else if (type == LightType::Directional)
            {
                const DirectionalLight* dl = static_cast<const DirectionalLight*>(light);
                glm::vec3 dir = dl->GetDirection();
                glm::quat rot = glm::rotation(glm::vec3(0.0f, 1.0f, 0.0f), glm::normalize(dir));
                model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
                model = model * glm::mat4_cast(rot);
                model = glm::scale(model, glm::vec3(1.0f, 3.0f, 1.0f));
            }

            PerObjectUBO ubo{};
            ubo.model = model;
            ubo.albedo = color;
            ubo.metallic = 0.0f;
            ubo.roughness = 1.0f;
            ubo.ao = 1.0f;
            ubo.normalStrength = 1.0f;
            ubo.parallaxMode = 0;
            ubo.parallaxScale = 0.0f;
            ubo.parallaxIterations = 0;

            uint32_t slot = m_resourceManager.AllocateObjectSlot();
            debugSlots.push_back(slot);
            m_resourceManager.UpdatePerObjectUBO(slot, ubo);

            uint32_t dynamicOffset = slot * m_resourceManager.GetObjectUBOStride();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);

            Mesh* mesh = nullptr;
            if (type == LightType::Point) mesh = &m_debugSphere;
            else if (type == LightType::Spot) mesh = &m_debugCone;
            else if (type == LightType::Directional) mesh = &m_debugArrow;

            if (mesh) mesh->Draw(cmd);
        }
        for (uint32_t slot : debugSlots)
        {
            m_resourceManager.FreeObjectSlot(slot);
        }
    }
#endif

    m_gui.Render(cmd);

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

    transitionImage(m_swapChain.GetSwapChainImages()[imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

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
        m_descriptorManager.UpdateGBufferDescriptorSet();
        m_descriptorManager.UpdateCompositeDescriptorSet();
        return;
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    float moveSpeed = 100.0f * m_deltaTime;
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

    m_gui.Shutdown();

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
    for (Light* light : m_scene.GetLights())
    {
        delete light;
    }
    m_scene.Destroy();
    m_mesh.Destroy();

#ifndef NDEBUG
    m_debugSphere.Destroy();
    m_debugCone.Destroy();
    m_debugArrow.Destroy();
#endif

    m_commandBufferManager.Shutdown();

    m_singleTexture.Cleanup();
    m_normalMap.Cleanup();
    m_heightMap.Cleanup();
    m_textureAtlas.Cleanup();

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

void Renderer::SetShowCursor(bool show)
{
    m_gui.SetShowCursor(show);
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