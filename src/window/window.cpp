#include "window/window.h"

Window::Window()
{

}

Window::Window(std::string title, int width, int height)
{
    m_title = title;
    m_width = width;
    m_height = height;

    Init();
}

Window::~Window()
{
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

    glfwMakeContextCurrent(m_window);

    m_renderer.Init(m_window);

    VkDevice device = m_renderer.GetDevice();
    if (device != VK_NULL_HANDLE)
        vkDeviceWaitIdle(device);
}

bool Window::ShouldActive()
{
    return !glfwWindowShouldClose(m_window);
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

void Window::FramebufferResizeCallback(GLFWwindow *window, int width, int height)
{
    auto win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
    win->m_renderer.SetFramebufferResized(true);
    win->m_width = width;
    win->m_height = height;
}
