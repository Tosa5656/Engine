#include "renderer/renderer.h"
#include <cmath>
#include <numeric>
#include <fstream>
#include <unistd.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <renderer/vulkan/light/directional.h>
#include <renderer/vulkan/light/point.h>
#include <renderer/vulkan/light/spot.h>
#include <renderer/vulkan/light/debug_mesh.h>

Renderer::Renderer() : m_destroyed(false) {}

Renderer::~Renderer()
{
    Destroy();
}

void Renderer::Init(VulkanContext* context, Surface* surface, GLFWwindow* window, Input* input)
{
    m_context = context;
    m_surface = surface;
    m_window = window;
    m_input = input;

    m_resourceManager.Create(&m_context->device, &m_swapChain, &m_context->instance);
    m_resourceManager.SetAllocator(m_context->allocator);
    m_swapChain.Create(&m_context->device, m_window, m_surface, m_context->allocator);
    m_swapChain.CreateImageViews(&m_context->device);
    m_descriptorManager.Init(&m_context->device, &m_swapChain, &m_resourceManager, 256);
    m_descriptorManager.CreateDescriptorSetLayout();
    m_pipelineManager.Create(&m_context->device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout(), m_descriptorManager.GetNormalMapSetLayout(), m_descriptorManager.GetHeightMapSetLayout(), m_descriptorManager.GetLightSetLayout());
    m_pipelineManager.CreateLinePipeline(&m_context->device, &m_swapChain);
    m_commandBufferManager.Init(m_context->device.GetDevice(), m_context->device.GetGraphicsQueueFamilyIndex(m_surface));
    m_resourceManager.CreateUniformBuffers();
    m_resourceManager.CreateObjectBuffer(128);
    m_resourceManager.CreateLightBuffers();

#ifndef NDEBUG
    {
        std::vector<MeshVertex> sphereVerts;
        std::vector<MeshIndex> sphereIndices;
        GenerateSphereMesh(sphereVerts, sphereIndices, 1.0f, 16, 12);
        m_debugSphere.Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), sphereVerts, sphereIndices);

        std::vector<MeshVertex> coneVerts;
        std::vector<MeshIndex> coneIndices;
        GenerateConeMesh(coneVerts, coneIndices, 1.0f, 1.0f, 16);
        m_debugCone.Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), coneVerts, coneIndices);

        std::vector<MeshVertex> arrowVerts;
        std::vector<MeshIndex> arrowIndices;
        GenerateArrowMesh(arrowVerts, arrowIndices, 1.0f, 0.3f, 0.15f);
        m_debugArrow.Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), arrowVerts, arrowIndices);
    }
#endif

    m_material.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    m_material2.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));
    m_material3.SetAlbedo(glm::vec3(1.0f, 1.0f, 1.0f));

    m_material4.SetAlbedo(glm::vec3(0.2f, 0.8f, 0.2f));
    m_material4.SetAlphaMode(AlphaMode::Blend);
    m_material4.SetMetallic(0.0f);
    m_material4.SetRoughness(0.3f);

    m_material5.SetAlbedo(glm::vec3(0.8f, 0.2f, 0.2f));
    m_material5.SetAlphaMode(AlphaMode::Cutoff);
    m_material5.SetAlphaCutoff(0.5f);
    m_material5.SetMetallic(0.0f);
    m_material5.SetRoughness(0.6f);

    m_material.Init(&m_context->device, m_resourceManager.GetAllocator());
    m_material2.Init(&m_context->device, m_resourceManager.GetAllocator());
    m_material3.Init(&m_context->device, m_resourceManager.GetAllocator());
    m_material4.Init(&m_context->device, m_resourceManager.GetAllocator());
    m_material5.Init(&m_context->device, m_resourceManager.GetAllocator());

    m_textureAtlas.Init(&m_context->device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool(), 16);
    m_textureAtlas.AddTexture("textures/BrickWall23_1K_BaseColor.png");
    m_textureAtlas.Build();

    m_singleTexture.Init(&m_context->device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_singleTexture.Load("textures/BrickWall23_1K_BaseColor.png");

    m_normalMap.Init(&m_context->device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
    m_normalMap.Load("textures/BrickWall23_1K_Normal.png");

    m_heightMap.Init(&m_context->device, m_resourceManager.GetAllocator(), m_commandBufferManager.GetCommandPool());
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
    obj1->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj1->SetMaterial(&m_material);
    obj1->GetTransform()->SetPosition(glm::vec3(0.0f, 0.0f, 0.0f));

    m_scene.AddObject(obj1);

    Object* obj2 = new Object();
    obj2->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj2->SetMaterial(&m_material2);
    obj2->GetTransform()->SetPosition(glm::vec3(3.0f, 0.0f, 0.0f));
    m_scene.AddObject(obj2);

    Object* obj3 = new Object();
    obj3->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    obj3->SetMaterial(&m_material3);
    obj3->GetTransform()->SetPosition(glm::vec3(-3.0f, 0.0f, 0.0f));
    m_scene.AddObject(obj3);

    Object* transparentObj = new Object();
    transparentObj->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    transparentObj->SetMaterial(&m_material4);
    transparentObj->GetTransform()->SetPosition(glm::vec3(0.0f, 4.0f, 0.0f));
    m_scene.AddObject(transparentObj);

    Object* cutoffObj = new Object();
    cutoffObj->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/cube.obj");
    cutoffObj->SetMaterial(&m_material5);
    cutoffObj->GetTransform()->SetPosition(glm::vec3(1.5f, 2.0f, 0.0f));
    m_scene.AddObject(cutoffObj);

    Object* plane = new Object();
    plane->Init(&m_context->device, &m_commandBufferManager, m_resourceManager.GetAllocator(), &m_resourceManager, "models/plane.obj");
    plane->GetTransform()->SetPosition(glm::vec3(0.0f, -2.0f, 0.0f));
    m_scene.AddObject(plane);

    m_scene.GetCamera()->SetPosition(glm::vec3(8.0f, 5.0f, 8.0f));
    m_scene.GetCamera()->SetAspectRatio(m_swapChain.GetSwapChainExtent().width / (float)m_swapChain.GetSwapChainExtent().height);

    DirectionalLight* sun = new DirectionalLight();
    sun->SetDirection(glm::vec3(1.0f, -1.0f, 0.5f));
    sun->SetColor(glm::vec3(1.0f, 0.95f, 0.8f));
    sun->SetIntensity(10.5f);
    sun->SetCastShadows(true);
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

    if (m_material4.HasTexture())
    {
        if (m_material4.GetTextureArray())
            m_material4.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material4.GetTextureArray()));
        else if (m_material4.GetTexture())
            m_material4.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material4.GetTexture()));
    }

    if (m_material5.HasTexture())
    {
        if (m_material5.GetTextureArray())
            m_material5.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material5.GetTextureArray()));
        else if (m_material5.GetTexture())
            m_material5.SetDescriptorSet(m_descriptorManager.CreateTextureDescriptorSet(m_material5.GetTexture()));
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

    {
        VkImageCreateInfo imgInfo{};
        imgInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imgInfo.imageType = VK_IMAGE_TYPE_2D;
        imgInfo.extent.width = m_shadowMapSize;
        imgInfo.extent.height = m_shadowMapSize;
        imgInfo.extent.depth = 1;
        imgInfo.mipLevels = 1;
        imgInfo.arrayLayers = 1;
        imgInfo.format = VK_FORMAT_D32_SFLOAT;
        imgInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imgInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imgInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        imgInfo.samples = VK_SAMPLE_COUNT_1_BIT;

        VmaAllocationCreateInfo allocInfo{};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

        if (vmaCreateImage(m_resourceManager.GetAllocator(), &imgInfo, &allocInfo, &m_shadowMapImage, &m_shadowMapAllocation, nullptr) != VK_SUCCESS)
            throw std::runtime_error("failed to create shadow map image!");
    }

    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_shadowMapImage;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = VK_FORMAT_D32_SFLOAT;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_context->device.GetDevice(), &viewInfo, nullptr, &m_shadowMapView) != VK_SUCCESS)
            throw std::runtime_error("failed to create shadow map image view!");
    }

    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_LESS;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;

        if (vkCreateSampler(m_context->device.GetDevice(), &samplerInfo, nullptr, &m_shadowSampler) != VK_SUCCESS)
            throw std::runtime_error("failed to create shadow sampler!");
    }

    m_descriptorManager.CreateShadowSetLayout();
    m_descriptorManager.CreateShadowDescriptorSet(m_shadowMapView, m_shadowSampler);

    m_descriptorManager.CreateGBufferDescriptorSet();
    m_descriptorManager.CreateCompositeDescriptorSet();
    m_descriptorManager.CreateHdrDescriptorSet();

    m_pipelineManager.CreateGBufferPipeline(&m_context->device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout(), m_descriptorManager.GetNormalMapSetLayout(), m_descriptorManager.GetHeightMapSetLayout());
    m_pipelineManager.CreateLightingPipeline(&m_context->device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetGBufferSetLayout(), m_descriptorManager.GetLightSetLayout(), m_descriptorManager.GetShadowSetLayout());
    m_pipelineManager.CreateCompositePipeline(&m_context->device, &m_swapChain, m_descriptorManager.GetCompositeSetLayout());
    m_pipelineManager.CreateTonemapPipeline(&m_context->device, &m_swapChain, m_descriptorManager.GetCompositeSetLayout(), m_descriptorManager.GetDescriptorSetLayout(0));

    m_resourceManager.CreateLuminanceBuffers();
    m_descriptorManager.CreateLuminanceSetLayout();
    m_descriptorManager.CreateLuminanceDescriptorSet();
    m_pipelineManager.CreateLuminancePipeline(&m_context->device, m_descriptorManager.GetLuminanceSetLayout());

    {
        VkExtent2D extent = m_swapChain.GetSwapChainExtent();
        uint32_t tileWidth = 32, tileHeight = 32;
        uint32_t tileCountX = (extent.width + tileWidth - 1) / tileWidth;
        uint32_t tileCountY = (extent.height + tileHeight - 1) / tileHeight;
        uint32_t depthSlices = 16;
        m_resourceManager.CreateClusterGrid(tileCountX, tileCountY, depthSlices);
    }

    m_descriptorManager.CreateClusterSetLayout();
    m_pipelineManager.CreateClusterCullPipeline(&m_context->device, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetClusterSetLayout());
    m_pipelineManager.CreateClusteredForwardPipeline(&m_context->device, &m_swapChain, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_descriptorManager.GetTextureSetLayout(), m_descriptorManager.GetNormalMapSetLayout(), m_descriptorManager.GetHeightMapSetLayout(), m_descriptorManager.GetClusterSetLayout());
    m_descriptorManager.CreateClusterDescriptorSet();

    m_pipelineManager.CreateShadowPipeline(&m_context->device, m_descriptorManager.GetDescriptorSetLayout(0), m_descriptorManager.GetDescriptorSetLayout(1), m_shadowMapSize);

    CreateSyncObjects();
    m_imagesInFlight.resize(m_swapChain.GetSwapChainImages().size(), VK_NULL_HANDLE);

    m_context->device.CreateTimestampQueryPool();

    m_gui.Init(m_context->device.GetDevice(), m_context->instance.GetInstance(), m_context->device.GetPhysicalDevice(), m_context->device.GetGraphicsQueueFamilyIndex(m_surface), m_context->device.GetGraphicsQueue(), m_window, VK_NULL_HANDLE, static_cast<uint32_t>(m_swapChain.GetSwapChainImages().size()));
}

void Renderer::Render()
{
    m_fpsAccumulator += m_deltaTime;
    m_fpsFrameCount++;
    if (m_fpsAccumulator >= 0.1f)
    {
        m_fps = static_cast<float>(m_fpsFrameCount) / m_fpsAccumulator;
        m_fpsAccumulator = 0.0f;
        m_fpsFrameCount = 0;
    }

    uint64_t gpuMemoryUsed, gpuMemoryBudget;
    m_resourceManager.GetMemoryBudget(gpuMemoryUsed, gpuMemoryBudget);
    
    uint64_t cpuMemory = 0;
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS memCounters;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &memCounters, sizeof(memCounters)))
    {
        cpuMemory = memCounters.WorkingSetSize;
    }
#else
    std::ifstream statm("/proc/self/statm");
    if (statm)
    {
        long pageSize = sysconf(_SC_PAGESIZE);
        long size, rss;
        statm >> size >> rss;
        cpuMemory = static_cast<uint64_t>(rss) * pageSize;
    }
#endif

    m_gui.NewFrame();
    m_gui.UpdateStats(m_deltaTime, gpuMemoryUsed, gpuMemoryBudget, cpuMemory);
    m_gui.DebugInfo(static_cast<int>(m_fps));

    {
        ImGui::Begin("Exposure Control");
        ImGui::Checkbox("Auto Exposure", &m_autoExposure);
        if (!m_autoExposure)
        {
            ImGui::SliderFloat("Exposure", &m_manualExposure, 0.01f, 10.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
        }
        else
        {
            ImGui::Text("Current: %.3f", m_smoothedExposure);
            ImGui::SliderFloat("Target Luminance", &m_targetLuminance, 0.01f, 1.0f, "%.3f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Min Exposure", &m_minExposure, 0.01f, 1.0f, "%.2f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Max Exposure", &m_maxExposure, 1.0f, 20.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
            ImGui::SliderFloat("Adapt Speed", &m_adaptSpeed, 0.1f, 10.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        }
        ImGui::End();
    }

    {
        ImGui::Begin("Shadow Controls");
        ImGui::SliderFloat("Depth Bias", &m_shadowBiasConstant, 0.0f, 0.05f, "%.5f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Slope Bias", &m_shadowBiasSlope, 0.0f, 5.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderInt("PCF Kernel", &m_shadowPcfKernel, 1, 3);
        ImGui::Text("Kernel: %dx%d (1=hardware, 3=manual 3x3)", m_shadowPcfKernel, m_shadowPcfKernel);
        ImGui::End();
    }

    {
        ImGui::Begin("Light Control");
        ImGui::SliderFloat("Yaw", &m_lightYaw, -180.0f, 180.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Pitch", &m_lightPitch, -90.0f, 90.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::SliderFloat("Intensity", &m_lightIntensity, 0.0f, 50.0f, "%.1f", ImGuiSliderFlags_AlwaysClamp);
        ImGui::Text("Direction: (%.2f, %.2f, %.2f)", m_lightDir.x, m_lightDir.y, m_lightDir.z);
        ImGui::End();
    }

    vkWaitForFences(m_context->device.GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    if (m_autoExposure && m_luminanceValid)
    {
        uint32_t histogram[256];
        m_resourceManager.ReadLuminanceHistogram(histogram);
        uint64_t totalPixels = std::accumulate(std::begin(histogram), std::end(histogram), uint64_t(0));
        if (totalPixels > 0)
        {
            uint64_t excludeLow  = totalPixels * 1 / 100;
            uint64_t excludeHigh = totalPixels * 1 / 100;
            uint64_t running = 0;
            double weightedSum = 0.0;
            uint64_t weightedCount = 0;
            bool started = false;
            for (int i = 0; i < 256; i++)
            {
                running += histogram[i];
                if (!started && running > excludeLow)
                    started = true;
                if (started)
                {
                    double logLuma = (double(i) + 0.5) * (32.0 / 256.0) - 16.0;
                    uint64_t w = histogram[i];
                    if (running >= excludeLow + excludeHigh && w > 0)
                    {
                        uint64_t clip = running - (totalPixels - excludeHigh);
                        if (clip < w) w -= clip;
                    }
                    weightedSum += logLuma * double(w);
                    weightedCount += w;
                }
                if (running >= totalPixels - excludeHigh)
                    break;
            }
            if (weightedCount > 0)
            {
                float avgLogLuma = float(weightedSum / double(weightedCount));
                float avgLuma = std::exp2(avgLogLuma);
                float targetExposure = m_targetLuminance / std::max(avgLuma, 1e-6f);
                targetExposure = std::clamp(targetExposure, m_minExposure, m_maxExposure);
                float rate = 1.0f - std::exp(-m_adaptSpeed * m_deltaTime);
                m_smoothedExposure = m_smoothedExposure + (targetExposure - m_smoothedExposure) * rate;
            }
        }
    }
    m_luminanceValid = false;

    if (!m_autoExposure)
        m_smoothedExposure = m_manualExposure;

    m_resourceManager.SetExposure(m_smoothedExposure);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        m_context->device.GetDevice(),
        m_swapChain.GetSwapChain(),
        UINT64_MAX,
        m_imageAvailableSemaphores[m_currentFrame],
        VK_NULL_HANDLE,
        &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR)
    {
        m_swapChain.Recreate(&m_context->device, m_window, m_surface, &m_commandBufferManager, m_resourceManager.GetAllocator());
        RecreatePerImageSemaphores();
        m_descriptorManager.UpdateGBufferDescriptorSet();
        m_descriptorManager.UpdateCompositeDescriptorSet();
        m_descriptorManager.UpdateClusterDescriptorSet();
        m_descriptorManager.UpdateHdrDescriptorSet();
        m_descriptorManager.UpdateLuminanceDescriptorSet();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    if (m_imagesInFlight[imageIndex] != VK_NULL_HANDLE)
    {
        vkWaitForFences(m_context->device.GetDevice(), 1, &m_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_imagesInFlight[imageIndex] = m_inFlightFences[m_currentFrame];

    for (Object* obj : m_scene.GetObjects())
    {
        if (obj && obj->IsActive())
        {
            obj->UpdateUBO(&m_resourceManager, m_scene.GetCamera()->GetPosition());
        }
    }

    glm::mat4 lightSpaceMatrix(1.0f);
    {
        auto& lights = m_scene.GetLights();
        if (!lights.empty() && lights[0] && lights[0]->GetType() == LightType::Directional)
        {
            DirectionalLight* sun = static_cast<DirectionalLight*>(lights[0]);
            float yawRad = glm::radians(m_lightYaw);
            float pitchRad = glm::radians(m_lightPitch);
            m_lightDir = glm::normalize(glm::vec3(
                cos(yawRad) * cos(pitchRad),
                sin(pitchRad),
                sin(yawRad) * cos(pitchRad)
            ));
            sun->SetDirection(m_lightDir);
            sun->SetIntensity(m_lightIntensity);

            glm::vec3 lightDir = glm::normalize(sun->GetDirection());
            glm::vec3 up = glm::abs(glm::dot(lightDir, glm::vec3(0.0f, 1.0f, 0.0f))) > 0.99f
                ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.0f, 1.0f, 0.0f);

            Camera* cam = m_scene.GetCamera();
            auto corners = cam->GetFrustumCorners();

            glm::vec3 center(0.0f);
            for (auto& c : corners)
                center += c;
            center /= 8.0f;

            float radius = glm::distance(corners[0], center);
            glm::vec3 lightPos = center - lightDir * radius * 2.0f;
            glm::mat4 lightView = glm::lookAt(lightPos, center, up);

            glm::vec3 sceneMin(1e30f);
            glm::vec3 sceneMax(-1e30f);
            for (auto& c : corners)
            {
                glm::vec4 ls = lightView * glm::vec4(c, 1.0f);
                sceneMin = glm::min(sceneMin, glm::vec3(ls));
                sceneMax = glm::max(sceneMax, glm::vec3(ls));
            }
            for (Object* obj : m_scene.GetObjects())
            {
                if (obj && obj->IsActive())
                {
                    glm::vec3 pos = obj->GetTransform()->GetPosition();
                    glm::vec4 lp = lightView * glm::vec4(pos, 1.0f);
                    sceneMin = glm::min(sceneMin, glm::vec3(lp) - 5.0f);
                    sceneMax = glm::max(sceneMax, glm::vec3(lp) + 5.0f);
                }
            }

            float zNear = std::max(0.1f, -sceneMax.z);
            float zFar = zNear + std::max(1.0f, -sceneMin.z - zNear);

            glm::mat4 lightProj = glm::ortho(sceneMin.x, sceneMax.x, sceneMin.y, sceneMax.y, zNear, zFar);
            lightProj[1][1] *= -1;
            lightSpaceMatrix = lightProj * lightView;
        }
    }
    m_scene.Update(m_deltaTime, &m_resourceManager);

    m_resourceManager.UpdatePerFrameUBO(imageIndex, *m_scene.GetCamera(), lightSpaceMatrix);

    VkCommandBuffer cmd = m_commandBufferManager.GetCommandBuffer(m_currentFrame);
    vkResetCommandBuffer(cmd, 0);

    VkCommandBufferBeginInfo beginInfo{ .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
    if (vkBeginCommandBuffer(cmd, &beginInfo) != VK_SUCCESS)
        throw std::runtime_error("failed to begin command buffer!");

    vkCmdResetQueryPool(cmd, m_context->device.GetTimestampQueryPool(), 0, 2);
    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, m_context->device.GetTimestampQueryPool(), 0);

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
        transitionImage(m_shadowMapImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT, 0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

        VkRenderingAttachmentInfo shadowDepth{};
        shadowDepth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        shadowDepth.imageView = m_shadowMapView;
        shadowDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
        shadowDepth.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        shadowDepth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        shadowDepth.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo shadowRendering{};
        shadowRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        shadowRendering.renderArea = {{0, 0}, {m_shadowMapSize, m_shadowMapSize}};
        shadowRendering.layerCount = 1;
        shadowRendering.colorAttachmentCount = 0;
        shadowRendering.pColorAttachments = nullptr;
        shadowRendering.pDepthAttachment = &shadowDepth;

        vkCmdBeginRendering(cmd, &shadowRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetShadowPipeline());

        VkViewport shadowVP{};
        shadowVP.x = 0.0f;
        shadowVP.y = 0.0f;
        shadowVP.width = (float)m_shadowMapSize;
        shadowVP.height = (float)m_shadowMapSize;
        shadowVP.minDepth = 0.0f;
        shadowVP.maxDepth = 1.0f;
        vkCmdSetViewport(cmd, 0, 1, &shadowVP);

        VkRect2D shadowScissor{};
        shadowScissor.offset = {0, 0};
        shadowScissor.extent = {m_shadowMapSize, m_shadowMapSize};
        vkCmdSetScissor(cmd, 0, 1, &shadowScissor);

        vkCmdSetDepthBias(cmd, m_shadowBiasConstant, 0.0f, m_shadowBiasSlope);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetShadowPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);

        for (Object* obj : m_scene.GetObjects())
        {
            if (!obj || !obj->IsActive() || obj->IsTransparent()) continue;

            uint32_t dynamicOffset = obj->GetUBOSlot() * m_resourceManager.GetObjectUBOStride();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetShadowPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);
            obj->Draw(cmd, m_descriptorManager.GetPerObjectDescriptorSets()[0], m_resourceManager.GetObjectUBOStride());
        }

        vkCmdEndRendering(cmd);

        transitionImage(m_shadowMapImage, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
    }

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
            if (!obj || !obj->IsActive() || obj->IsTransparent()) continue;

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

        transitionImage(gbufferImages[4], VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_DEPTH_BIT, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT,
            VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

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

        VkDescriptorSet shadowDS = m_descriptorManager.GetShadowDescriptorSet();
        if (shadowDS != VK_NULL_HANDLE)
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetLightingPipelineLayout(), 3, 1, &shadowDS, 0, nullptr);

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

        transitionImage(m_swapChain.GetHdrColorImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    {
        VkRenderingAttachmentInfo compositeAttachment{};
        compositeAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        compositeAttachment.imageView = m_swapChain.GetHdrColorImageView();
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

        vkCmdEndRendering(cmd);
    }

    {
        VkDescriptorSet clusterDS = m_descriptorManager.GetClusterDescriptorSet();
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineManager.GetClusterCullPipeline());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineManager.GetClusterCullPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineManager.GetClusterCullPipelineLayout(), 1, 1, &clusterDS, 0, nullptr);
        uint32_t groupCount = m_resourceManager.GetClusterCount();
        vkCmdDispatch(cmd, groupCount, 1, 1);

        VkMemoryBarrier clusterBarrier{};
        clusterBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        clusterBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        clusterBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 1, &clusterBarrier, 0, nullptr, 0, nullptr);
    }

    {
        VkRenderingAttachmentInfo forwardAttachment{};
        forwardAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        forwardAttachment.imageView = m_swapChain.GetHdrColorImageView();
        forwardAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        forwardAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        forwardAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingAttachmentInfo forwardDepth{};
        forwardDepth.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        forwardDepth.imageView = m_swapChain.GetDepthImageView();
        forwardDepth.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        forwardDepth.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        forwardDepth.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        forwardDepth.clearValue.depthStencil = {1.0f, 0};

        VkRenderingInfo forwardRendering{};
        forwardRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        forwardRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        forwardRendering.layerCount = 1;
        forwardRendering.colorAttachmentCount = 1;
        forwardRendering.pColorAttachments = &forwardAttachment;
        forwardRendering.pDepthAttachment = &forwardDepth;

        vkCmdBeginRendering(cmd, &forwardRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipeline());
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 0, 1, &m_descriptorManager.GetDescriptorSets()[imageIndex], 0, nullptr);
        VkDescriptorSet clusterDS = m_descriptorManager.GetClusterDescriptorSet();

        for (Object* obj : m_scene.GetObjects())
        {
            if (!obj || !obj->IsActive() || !obj->IsTransparent()) continue;

            uint32_t dynamicOffset = obj->GetUBOSlot() * m_resourceManager.GetObjectUBOStride();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 1, 1, m_descriptorManager.GetPerObjectDescriptorSets().data(), 1, &dynamicOffset);

            Material* material = obj->GetMaterial();
            VkDescriptorSet texDS = (material && material->HasTexture() && material->GetDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetDescriptorSet() : m_descriptorManager.GetNullTextureDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 2, 1, &texDS, 0, nullptr);

            VkDescriptorSet normDS = (material && material->GetNormalMapDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetNormalMapDescriptorSet() : m_descriptorManager.GetNullNormalMapDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 3, 1, &normDS, 0, nullptr);

            VkDescriptorSet heightDS = (material && material->GetHeightMapDescriptorSet() != VK_NULL_HANDLE)
                ? material->GetHeightMapDescriptorSet() : m_descriptorManager.GetNullHeightMapDescriptorSet();
            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 4, 1, &heightDS, 0, nullptr);

            vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetClusteredForwardPipelineLayout(), 5, 1, &clusterDS, 0, nullptr);

            obj->Draw(cmd, m_descriptorManager.GetPerObjectDescriptorSets()[0], m_resourceManager.GetObjectUBOStride());
        }

        vkCmdEndRendering(cmd);
    }

    {
        transitionImage(m_swapChain.GetHdrColorImage(), VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
            VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        transitionImage(m_swapChain.GetSwapChainImages()[imageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            VK_IMAGE_ASPECT_COLOR_BIT, 0, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
    }

    {
        VkBuffer luminanceBuf = m_resourceManager.GetLuminanceStorageBuffer();
        VkBuffer stagingBuf = m_resourceManager.GetLuminanceStagingBuffer();

        vkCmdFillBuffer(cmd, luminanceBuf, 0, sizeof(uint32_t) * 256, 0);

        VkBufferMemoryBarrier fillBarrier{};
        fillBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        fillBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        fillBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        fillBarrier.buffer = luminanceBuf;
        fillBarrier.offset = 0;
        fillBarrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, 0, 0, nullptr, 1, &fillBarrier, 0, nullptr);

        VkDescriptorSet luminanceDS = m_descriptorManager.GetLuminanceDescriptorSet();
        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineManager.GetLuminancePipeline());
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineManager.GetLuminancePipelineLayout(), 0, 1, &luminanceDS, 0, nullptr);
        VkExtent2D extent = m_swapChain.GetSwapChainExtent();
        uint32_t groupX = (extent.width + 15) / 16;
        uint32_t groupY = (extent.height + 15) / 16;
        vkCmdDispatch(cmd, groupX, groupY, 1);

        VkBufferMemoryBarrier computeBarrier{};
        computeBarrier.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        computeBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        computeBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        computeBarrier.buffer = luminanceBuf;
        computeBarrier.offset = 0;
        computeBarrier.size = VK_WHOLE_SIZE;
        vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 1, &computeBarrier, 0, nullptr);

        VkBufferCopy copyRegion{};
        copyRegion.size = sizeof(uint32_t) * 256;
        vkCmdCopyBuffer(cmd, luminanceBuf, stagingBuf, 1, &copyRegion);

        m_luminanceValid = true;
    }

    {
        VkRenderingAttachmentInfo tonemapAttachment{};
        tonemapAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        tonemapAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
        tonemapAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        tonemapAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        tonemapAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo tonemapRendering{};
        tonemapRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        tonemapRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        tonemapRendering.layerCount = 1;
        tonemapRendering.colorAttachmentCount = 1;
        tonemapRendering.pColorAttachments = &tonemapAttachment;
        tonemapRendering.pDepthAttachment = nullptr;

        vkCmdBeginRendering(cmd, &tonemapRendering);

        vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetTonemapPipeline());
        vkCmdSetViewport(cmd, 0, 1, &viewport);
        vkCmdSetScissor(cmd, 0, 1, &scissor);

        VkDescriptorSet hdrDS = m_descriptorManager.GetHdrDescriptorSet();
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetTonemapPipelineLayout(), 0, 1, &hdrDS, 0, nullptr);
        VkDescriptorSet perFrameDS = m_descriptorManager.GetDescriptorSets()[imageIndex];
        vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineManager.GetTonemapPipelineLayout(), 1, 1, &perFrameDS, 0, nullptr);

        vkCmdDraw(cmd, 3, 1, 0, 0);

        vkCmdEndRendering(cmd);
    }

#ifndef NDEBUG
    if (m_debugLightsEnabled)
    {
        VkRenderingAttachmentInfo debugAttachment{};
        debugAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        debugAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
        debugAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        debugAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        debugAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo debugRendering{};
        debugRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        debugRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        debugRendering.layerCount = 1;
        debugRendering.colorAttachmentCount = 1;
        debugRendering.pColorAttachments = &debugAttachment;
        debugRendering.pDepthAttachment = nullptr;

        vkCmdBeginRendering(cmd, &debugRendering);

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
            ubo.alphaCutoff = 0.5f;
            ubo.alphaMode = 0;

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

        vkCmdEndRendering(cmd);
    }
#endif

    {
        VkRenderingAttachmentInfo imguiAttachment{};
        imguiAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
        imguiAttachment.imageView = m_swapChain.GetSwapChainImageViews()[imageIndex];
        imguiAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        imguiAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        imguiAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

        VkRenderingInfo imguiRendering{};
        imguiRendering.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
        imguiRendering.renderArea = {{0, 0}, m_swapChain.GetSwapChainExtent()};
        imguiRendering.layerCount = 1;
        imguiRendering.colorAttachmentCount = 1;
        imguiRendering.pColorAttachments = &imguiAttachment;
        imguiRendering.pDepthAttachment = nullptr;

        vkCmdBeginRendering(cmd, &imguiRendering);
        m_gui.Render(cmd);
        vkCmdEndRendering(cmd);
    }

    transitionImage(m_swapChain.GetSwapChainImages()[imageIndex],
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);

    vkCmdWriteTimestamp(cmd, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, m_context->device.GetTimestampQueryPool(), 1);

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

    if (vkResetFences(m_context->device.GetDevice(), 1, &m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to reset fence!");

    if (vkQueueSubmit(m_context->device.GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
        throw std::runtime_error("failed to submit draw command buffer!");

    vkWaitForFences(m_context->device.GetDevice(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    uint64_t timestamps[2];
    vkGetQueryPoolResults(
        m_context->device.GetDevice(),
        m_context->device.GetTimestampQueryPool(),
        0,
        2,
        sizeof(timestamps),
        timestamps,
        sizeof(uint64_t),
        VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);

    uint64_t deltaTicks = timestamps[1] - timestamps[0];
    m_deltaTime = static_cast<float>(deltaTicks * m_context->device.GetTimestampPeriod()) / 1e9f;

    VkSwapchainKHR swapChain = m_swapChain.GetSwapChain();

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(m_context->device.GetPresentQueue(), &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_framebufferResized)
    {
        m_framebufferResized = false;
        m_swapChain.Recreate(&m_context->device, m_window, m_surface, &m_commandBufferManager, m_resourceManager.GetAllocator());
        RecreatePerImageSemaphores();
        m_descriptorManager.UpdateGBufferDescriptorSet();
        m_descriptorManager.UpdateCompositeDescriptorSet();
        m_descriptorManager.UpdateClusterDescriptorSet();
        m_descriptorManager.UpdateHdrDescriptorSet();
        m_descriptorManager.UpdateLuminanceDescriptorSet();
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
        float rotateSpeed = 10 * m_deltaTime;
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
    if (m_destroyed)
        return;
    m_destroyed = true;

    VkDevice device = m_context->device.GetDevice();
    VkInstance instance = m_context->instance.GetInstance();

    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);

    m_gui.Shutdown();

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
        if (obj) obj->Destroy();
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

    m_descriptorManager.Cleanup();
    m_pipelineManager.Shutdown(&m_context->device);
    m_swapChain.Cleanup(&m_context->device);

    m_context->device.DestroyTimestampQueryPool();

    m_mesh.DestroyUploadFence();
#ifndef NDEBUG
    m_debugSphere.DestroyUploadFence();
    m_debugCone.DestroyUploadFence();
    m_debugArrow.DestroyUploadFence();
#endif

    if (m_shadowSampler != VK_NULL_HANDLE)
    {
        vkDestroySampler(device, m_shadowSampler, nullptr);
        m_shadowSampler = VK_NULL_HANDLE;
    }
    if (m_shadowMapView != VK_NULL_HANDLE)
    {
        vkDestroyImageView(device, m_shadowMapView, nullptr);
        m_shadowMapView = VK_NULL_HANDLE;
    }
    if (m_shadowMapImage != VK_NULL_HANDLE && m_shadowMapAllocation != VK_NULL_HANDLE)
    {
        vmaDestroyImage(m_resourceManager.GetAllocator(), m_shadowMapImage, m_shadowMapAllocation);
        m_shadowMapImage = VK_NULL_HANDLE;
        m_shadowMapAllocation = VK_NULL_HANDLE;
    }
}

void Renderer::SetFramebufferResized(bool resized)
{
    m_framebufferResized = resized;
}

VkDevice Renderer::GetDevice()
{
    return m_context->device.GetDevice();
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
        if (vkCreateSemaphore(m_context->device.GetDevice(), &semInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_context->device.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create synchronization objects!");
        }
    }

    for (size_t i = 0; i < imageCount; i++)
    {
        if (vkCreateSemaphore(m_context->device.GetDevice(), &semInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
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
        if (vkCreateSemaphore(m_context->device.GetDevice(), &semInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_context->device.GetDevice(), &semInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create semaphore for swapchain image!");
        }
    }
}

void Renderer::RecreatePerImageSemaphores()
{
    VkDevice device = m_context->device.GetDevice();
    for (auto& sem : m_renderFinishedSemaphores)
    {
        if (sem != VK_NULL_HANDLE)
            vkDestroySemaphore(device, sem, nullptr);
    }
    uint32_t imageCount = static_cast<uint32_t>(m_swapChain.GetSwapChainImages().size());
    m_renderFinishedSemaphores.resize(imageCount, VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semInfo{};
    semInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    for (uint32_t i = 0; i < imageCount; i++)
    {
        if (vkCreateSemaphore(device, &semInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS)
            throw std::runtime_error("failed to recreate render finished semaphore!");
    }
    m_imagesInFlight.resize(imageCount, VK_NULL_HANDLE);
}