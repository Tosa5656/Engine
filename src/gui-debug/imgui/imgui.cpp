#include "imgui.h"
#include <cstring>

void GUI::Init(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, GLFWwindow* window, VkRenderPass renderPass)
{
    m_device = device;
    m_instance = instance;
    m_physicalDevice = physicalDevice;
    m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    m_graphicsQueue = graphicsQueue;
    m_window = window;
    m_renderPass = renderPass;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;

    ImGui_ImplGlfw_InitForVulkan(window, true);

    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = m_instance;
    initInfo.PhysicalDevice = m_physicalDevice;
    initInfo.Device = m_device;
    initInfo.QueueFamily = m_graphicsQueueFamilyIndex;
    initInfo.Queue = m_graphicsQueue;
    initInfo.DescriptorPool = VK_NULL_HANDLE;
    initInfo.DescriptorPoolSize = 1000;
    initInfo.MinImageCount = 2;
    initInfo.ImageCount = 2;
    initInfo.CheckVkResultFn = nullptr;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
    VkFormat format = VK_FORMAT_B8G8R8A8_SRGB;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &format;
    initInfo.PipelineInfoMain.PipelineRenderingCreateInfo.depthAttachmentFormat = VK_FORMAT_UNDEFINED;
    
    ImGui_ImplVulkan_Init(&initInfo);
}

void GUI::NewFrame()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::DebugConsole()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Console");

    ImGui::End();
}

void GUI::DebugInfo(int fps)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Debug Info");

    ImGui::Text("FPS: %d", fps);
    ImGui::Text("Frame Time: %.2f ms", m_frameTime);
    ImGui::Text("Delta Time: %.4f s", m_deltaTime);
    ImGui::Separator();
    ImGui::Text("GPU Memory: %.2f MB", m_gpuMemory / 1024.0f / 1024.0f);
    ImGui::Text("CPU Memory: %.2f MB", m_cpuMemory / 1024.0f / 1024.0f);
    ImGui::Separator();
    ImGui::Text("Draw Calls: %d", m_drawCalls);

    ImGui::End();
}

void GUI::UpdateStats(float deltaTime, uint64_t gpuMemory, uint64_t cpuMemory)
{
    m_deltaTime = deltaTime;
    m_frameTime = deltaTime * 1000.0f;
    m_fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    m_gpuMemory = gpuMemory;
    m_cpuMemory = cpuMemory;
    m_drawCalls = ImGui::GetDrawData() ? ImGui::GetDrawData()->CmdListsCount : 0;
}

void GUI::Render(VkCommandBuffer commandBuffer)
{
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void GUI::Shutdown()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void GUI::SetShowCursor(bool show)
{
    ImGuiIO& io = ImGui::GetIO();
    if (!show)
    {
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    }
    else
    {
        io.ConfigFlags &= ~ImGuiConfigFlags_NoMouseCursorChange;
    }
}