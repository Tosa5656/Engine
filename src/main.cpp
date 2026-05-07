#include <iostream>
#include <window/window.h>

int main()
{
    Window window = Window("Singularity Engine", 1280, 720);

    while (window.ShouldActive())
    {
        glfwPollEvents();
        window.Update();

        if (window.GetInput()->IsPressed(KeyCode::Escape))
            window.GetInput()->SetCursorCaptured(false);
    }

    DestroyGLFW();
}