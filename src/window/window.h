#pragma once

#include <iostream>
#include <functional>

#include <GLFW/glfw3.h>

#include <renderer/renderer.h>
#include <renderer/vulkan/surface.h>
#include <renderer/vulkan/vulkan_context.h>
#include <utils/input/input.h>

class Window
{
public:
    Window();
    Window(std::string title, int width, int height, std::function<void()> awake, std::function<void()> start, std::function<void()> update, Window* rootWindow = nullptr);
    ~Window();

    void Update();
    void Close();

    bool ShouldActive();

    void SetRootWindow(Window* root, int offsetX = 0, int offsetY = 0);

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
    static void WindowCloseCallback(GLFWwindow* window);

    std::function<void()> m_awake;
    std::function<void()> m_start;
    std::function<void()> m_update;

    GLFWwindow* m_window;
    bool m_windowClosed;
    std::string m_title;
    int m_width;
    int m_height;

    bool m_cursorLocked = false;

    Window* m_rootWindow = nullptr;
    int m_rootOffsetX = 0;
    int m_rootOffsetY = 0;
    int m_lastRootX = 0;
    int m_lastRootY = 0;

    VulkanContext* m_context = nullptr;
    Surface m_surface;
    Renderer m_renderer;
    Input m_input;
};