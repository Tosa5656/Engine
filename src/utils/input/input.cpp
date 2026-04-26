#include "Input.h"

Input::Input(GLFWwindow* win) : window(win) {}

void Input::Update() {
    for (int key = 0; key < 512; key++) {
        previousState[key] = currentState[key];
        currentState[key] = glfwGetKey(window, key) == GLFW_PRESS;
    }
}

bool Input::IsDown(int key) {
    return glfwGetKey(window, key) == GLFW_PRESS;
}

bool Input::IsUp(int key) {
    return glfwGetKey(window, key) == GLFW_RELEASE;
}

bool Input::IsPressed(int key) {
    return currentState[key] && !previousState[key];
}

bool Input::IsReleased(int key) {
    return !currentState[key] && previousState[key];
}