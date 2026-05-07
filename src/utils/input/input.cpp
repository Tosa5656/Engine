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

    if (m_cursorCaptured)
    {
        m_previousMousePos = m_currentMousePos;
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        m_currentMousePos = glm::vec2(static_cast<float>(x), static_cast<float>(y));
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

glm::vec2 Input::GetMousePosition()
{
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    return glm::vec2(static_cast<float>(x), static_cast<float>(y));
}

glm::vec2 Input::GetMouseDelta()
{
    glm::vec2 delta = m_currentMousePos - m_previousMousePos;
    m_previousMousePos = m_currentMousePos;
    return delta;
}

bool Input::IsCursorCaptured()
{
    return m_cursorCaptured;
}

void Input::SetCursorCaptured(bool captured)
{
    m_cursorCaptured = captured;
    if (captured)
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    else
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}