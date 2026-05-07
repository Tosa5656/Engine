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

    void Update();
    void Close();

    bool ShouldActive();

    GLFWwindow* GetWindow();
    std::string GetTitle();
    int GetWidth();
    int GetHeight();
    Renderer* GetRenderer();
    Input* GetInput();
private:
    void Init();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    GLFWwindow* m_window;
    bool m_windowClosed;
    std::string m_title;
    int m_width;
    int m_height;

    Renderer m_renderer;
    Input m_input;
};