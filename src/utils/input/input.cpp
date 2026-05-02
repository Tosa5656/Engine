#include "input.h"

Input::Input() : window(nullptr) {}

Input::Input(GLFWwindow* win) : window(win) {}

void Input::Update()
{
    for (int key = static_cast<int>(KeyCode::Space); key <= static_cast<int>(KeyCode::Menu); key++)
    {
        previousState[key] = currentState[key];
        currentState[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }

    for (int button = static_cast<int>(MouseKeyCode::MouseLeft); button <= static_cast<int>(MouseKeyCode::Mouse8); button++)
    {
        previousMouseState[button] = currentMouseState[button];
        currentMouseState[button] = glfwGetMouseButton(window, button) == GLFW_PRESS;
    }
}

bool Input::IsPressed(KeyCode key)
{
    return glfwGetKey(window, static_cast<int>(key)) == GLFW_PRESS;
}

bool Input::IsReleased(KeyCode key)
{
    return glfwGetKey(window, static_cast<int>(key)) == GLFW_RELEASE;
}

bool Input::IsUp(KeyCode key)
{
    int k = static_cast<int>(key);
    return currentState[k] && !previousState[k];
}

bool Input::IsDown(KeyCode key)
{
    int k = static_cast<int>(key);
    return !currentState[k] && previousState[k];
}

bool Input::IsMousePressed(MouseKeyCode button)
{
    return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_PRESS;
}

bool Input::IsMouseReleased(MouseKeyCode button)
{
    return glfwGetMouseButton(window, static_cast<int>(button)) == GLFW_RELEASE;
}

bool Input::IsMouseUp(MouseKeyCode button)
{
    int b = static_cast<int>(button);
    return currentMouseState[b] && !previousMouseState[b];
}

bool Input::IsMouseDown(MouseKeyCode button)
{
    int b = static_cast<int>(button);
    return !currentMouseState[b] && previousMouseState[b];
}