#include <iostream>
#include <window/window.h>

bool space_state = false;

int main()
{
    Window window = Window("Singularity Engine", 1280, 720);

    while (window.ShouldActive())
    {
        glfwPollEvents();
        window.Update();

        if (window.GetInput()->IsPressed(KeyCode::Space))
        {
            window.GetInput()->SetCursorCaptured(space_state);
            window.GetRenderer()->SetShowCursor(!space_state);
            space_state = !space_state;
        }
    }

    DestroyGLFW();
}