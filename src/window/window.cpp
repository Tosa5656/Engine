#include "window/window.h"

Window::Window()
{
    m_window = nullptr;
    m_windowClosed = false;
}

Window::Window(std::string title, int width, int height)
{
    m_title = title;
    m_width = width;
    m_height = height;
    m_windowClosed = false;

    Init();
}

Window::~Window()
{
    Close();
    m_renderer.Destroy();
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

    glfwMakeContextCurrent(m_window);

    m_input = Input(m_window);
    m_renderer.Init(m_window, &m_input);

    VkDevice device = m_renderer.GetDevice();
    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);
}

void Window::Update()
{
    m_input.Update();
    m_renderer.Render();
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
    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
    {
        win->GetInput()->SetCursorCaptured(true);
        win->GetRenderer()->SetShowCursor(false);
    }
}
