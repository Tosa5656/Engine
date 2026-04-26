#pragma once

#include <GLFW/glfw3.h>
#include <unordered_map>

class Input {
private:
    GLFWwindow* window;
    std::unordered_map<int, bool> previousState;
    std::unordered_map<int, bool> currentState;
    
    void Update();
    
public:
    Input(GLFWwindow* win);
    
    bool IsDown(int key);
    bool IsUp(int key);
    bool IsPressed(int key);
    bool IsReleased(int key);
};