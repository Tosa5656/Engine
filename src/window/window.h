#pragma once

#include <iostream>
#include <functional>

#include <GLFW/glfw3.h>

#include <renderer/renderer.h>
#include <utils/input/input.h>

class Window
{
public:
    Window();
    Window(std::string title, int width, int height, std::function<void()> awake, std::function<void()> start, std::function<void()> update);
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
    float GetDeltaTime();
private:
    void Init();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
    static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    std::function<void()> m_awake;
    std::function<void()> m_start;
    std::function<void()> m_update;

    GLFWwindow* m_window;
    bool m_windowClosed;
    std::string m_title;
    int m_width;
    int m_height;

    Renderer m_renderer;
    Input m_input;
};