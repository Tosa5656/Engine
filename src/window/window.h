#pragma once

#include <iostream>

#include <GLFW/glfw3.h>

#include <renderer/renderer.h>

class Window
{
public:
    Window();
    Window(std::string title, int width, int height);
    ~Window();
private:
    void Init();

    GLFWwindow* m_window;
    std::string m_title;
    int m_width;
    int m_height;

    Renderer m_renderer;
};