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

}

void Window::Init()
{
    InitGLFW();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    m_window = glfwCreateWindow(m_width, m_height, m_title.c_str(), NULL, NULL);

    if (!m_window)
        return;

    glfwMakeContextCurrent(m_window);

    m_renderer.Init(m_window);

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
        m_renderer.Draw();
    }

    vkDeviceWaitIdle(m_renderer.GetDevice());
}
