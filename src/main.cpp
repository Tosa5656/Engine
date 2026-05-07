#include <iostream>
#include <window/window.h>

int main()
{
    Window window = Window("Engine", 800, 600);

    while (window.ShouldActive())
    {
        glfwPollEvents();
        window.GetRenderer()->Render();
    }

    DestroyGLFW();
}