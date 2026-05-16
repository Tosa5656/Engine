#include "imgui.h"
#include <cstring>

void GUI::Init(VkDevice device, VkInstance instance, VkPhysicalDevice physicalDevice, uint32_t graphicsQueueFamilyIndex, VkQueue graphicsQueue, GLFWwindow* window, VkRenderPass renderPass, uint32_t imageCount)
{
    m_device = device;
    m_instance = instance;
    m_physicalDevice = physicalDevice;
    m_graphicsQueueFamilyIndex = graphicsQueueFamilyIndex;
    m_graphicsQueue = graphicsQueue;
    m_window = window;
    m_renderPass = renderPass;

    IMGUI_CHECKVERSION();
    m_context = ImGui::CreateContext();
    ImGui::SetCurrentContext(m_context);

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
    initInfo.ImageCount = imageCount;
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
    ImGui::SetCurrentContext(m_context);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void GUI::DebugConsole()
{
    ImGui::SetCurrentContext(m_context);
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Console");

    ImGui::End();
}

void GUI::DebugInfo(int fps)
{
    ImGui::SetCurrentContext(m_context);
    ImGui::Begin("Debug Info");

    ImGui::Text("FPS: %d", fps);
    ImGui::Text("Frame Time: %.2f ms", m_frameTime);
    ImGui::Text("Delta Time: %.4f s", m_deltaTime);
    ImGui::Separator();
    ImGui::Text("GPU Memory: %.0f / %.0f MB", m_gpuMemory / 1024.0f / 1024.0f, m_gpuMemoryBudget / 1024.0f / 1024.0f);
    ImGui::Text("RAM: %.0f MB", m_cpuMemory / 1024.0f / 1024.0f);
    ImGui::Separator();
    ImGui::Text("Draw Calls: %d", m_drawCalls);

    ImGui::End();
}

void GUI::UpdateStats(float deltaTime, uint64_t gpuMemory, uint64_t gpuMemoryBudget, uint64_t cpuMemory)
{
    ImGui::SetCurrentContext(m_context);
    m_deltaTime = deltaTime;
    m_frameTime = deltaTime * 1000.0f;
    m_fps = deltaTime > 0.0f ? 1.0f / deltaTime : 0.0f;
    m_gpuMemory = gpuMemory;
    m_gpuMemoryBudget = gpuMemoryBudget;
    m_cpuMemory = cpuMemory;
}

void GUI::Render(VkCommandBuffer commandBuffer)
{
    ImGui::SetCurrentContext(m_context);
    ImGui::Render();
    m_drawCalls = ImGui::GetDrawData() ? ImGui::GetDrawData()->CmdListsCount : 0;
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void GUI::Shutdown()
{
    ImGui::SetCurrentContext(m_context);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(m_context);
}

void GUI::SetShowCursor(bool show)
{
    ImGui::SetCurrentContext(m_context);
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