#include "window/window.h"

Window::Window()
{
    m_window = nullptr;
    m_windowClosed = false;
}

Window::Window(std::string title, int width, int height, std::function<void()> awake, std::function<void()> start, std::function<void()> update, Window* rootWindow)
{
    m_title = title;
    m_width = width;
    m_height = height;
    m_windowClosed = false;

    m_awake = awake;
    m_start = start;
    m_update = update;

    m_rootWindow = rootWindow;

    Init();

    if (rootWindow)
    {
        glfwGetWindowPos(rootWindow->m_window, &m_lastRootX, &m_lastRootY);
        glfwSetWindowPos(m_window, m_lastRootX + m_rootOffsetX, m_lastRootY + m_rootOffsetY);
    }
}

Window::~Window()
{
    m_renderer.Destroy();

    if (m_context)
        m_surface.Cleanup(&m_context->instance);

    if (m_window != nullptr && !m_windowClosed)
    {
        m_windowClosed = true;
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    DestroyGLFW();

    if (m_context && glfw_refcount == 0)
        VulkanContext::DestroyInstance();
}

void Window::Init()
{
    InitGLFW();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);

    if (!m_window)
        return;

    glfwSetWindowUserPointer(m_window, this);
    glfwSetFramebufferSizeCallback(m_window, FramebufferResizeCallback);
    glfwSetKeyCallback(m_window, KeyCallback);
    glfwSetWindowCloseCallback(m_window, WindowCloseCallback);

    m_awake();

    glfwSwapInterval(0);

    m_start();

    m_input = Input(m_window);

    m_context = VulkanContext::Get();
    if (!m_context)
    {
        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Program";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Singularity Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_4;

        m_context = VulkanContext::Create(appInfo);
    }

    m_surface.Create(&m_context->instance, m_window);

    if (m_context->allocator == VK_NULL_HANDLE)
    {
        m_context->device.PickPhysicalDevice(&m_context->instance, &m_surface);
        m_context->device.Create(&m_context->instance, &m_surface);

        VmaAllocatorCreateInfo allocatorInfo{};
        allocatorInfo.physicalDevice = m_context->device.GetPhysicalDevice();
        allocatorInfo.device = m_context->device.GetDevice();
        allocatorInfo.instance = m_context->instance.GetInstance();
        if (vmaCreateAllocator(&allocatorInfo, &m_context->allocator) != VK_SUCCESS)
            throw std::runtime_error("failed to create VMA allocator!");
    }

    m_renderer.Init(m_context, &m_surface, m_window, &m_input);

    VkDevice device = m_context->device.GetDevice();
    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);
}

void Window::Update()
{
    m_input.Update();
    glfwPollEvents();

    if (m_rootWindow)
    {
        int rx, ry;
        glfwGetWindowPos(m_rootWindow->m_window, &rx, &ry);
        if (rx != m_lastRootX || ry != m_lastRootY)
        {
            m_lastRootX = rx;
            m_lastRootY = ry;
            glfwSetWindowPos(m_window, rx + m_rootOffsetX, ry + m_rootOffsetY);
        }
    }

    m_renderer.Render();
    m_update();
}

void Window::Close()
{
    if (m_window != nullptr && !m_windowClosed)
    {
        m_windowClosed = true;
        
        VkDevice device = m_renderer.GetDevice();
        if (device != VK_NULL_HANDLE)
            vkDeviceWaitIdle(device);

        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
}

bool Window::ShouldActive()
{
    return m_window != nullptr && !glfwWindowShouldClose(m_window);
}

GLFWwindow *Window::GetWindow()
{
    return m_window;
}

std::string Window::GetTitle()
{
    return m_title;
}

int Window::GetWidth()
{
    return m_width;
}

int Window::GetHeight()
{
    return m_height;
}

Renderer *Window::GetRenderer()
{
    return &m_renderer;
}

Input *Window::GetInput()
{
    return &m_input;
}

float Window::GetDeltaTime()
{
    return m_renderer.GetDeltaTime();
}

void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    win->m_renderer.SetFramebufferResized(true);
    win->m_width = width;
    win->m_height = height;
}

void Window::KeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    win->GetInput()->OnKeyEvent(key, action);
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        win->m_cursorLocked = !win->m_cursorLocked;
        win->GetInput()->SetCursorCaptured(win->m_cursorLocked);
        win->GetRenderer()->SetShowCursor(!win->m_cursorLocked);
    }
}

void Window::WindowCloseCallback(GLFWwindow* window)
{
    glfwHideWindow(window);
}

void Window::SetRootWindow(Window* root, int offsetX, int offsetY)
{
    m_rootWindow = root;
    m_rootOffsetX = offsetX;
    m_rootOffsetY = offsetY;

    if (root && m_window)
    {
        glfwGetWindowPos(root->m_window, &m_lastRootX, &m_lastRootY);
        glfwSetWindowPos(m_window, m_lastRootX + offsetX, m_lastRootY + offsetY);
    }
}
