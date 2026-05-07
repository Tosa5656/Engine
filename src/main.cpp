#include <iostream>
#include <window/window.h>

int main()
{
    Window window = Window("Singularity Engine", 800, 600);

    while (window.ShouldActive())
    {
        glfwPollEvents();
        window.Update();

        if (window.GetInput()->IsPressed(KeyCode::Escape))
            window.GetInput()->SetCursorCaptured(false);
    }

    DestroyGLFW();
}