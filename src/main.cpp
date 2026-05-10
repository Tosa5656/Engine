#include <iostream>
#include <window/window.h>

Window* window_ptr;
bool space_state = false;

void Awake()
{

}

void Start()
{

}

void Update()
{
    if (window_ptr->GetInput()->IsDown(KeyCode::Space))
    {
        window_ptr->GetInput()->SetCursorCaptured(space_state);
        window_ptr->GetRenderer()->SetShowCursor(!space_state);
        space_state = !space_state;
    }
}

int main()
{
    Window window = Window("Singularity Engine", 1280, 720, Awake, Start, Update);
    window_ptr = &window;

    while (window.ShouldActive())
    {
        glfwPollEvents();
        window.Update();
    }

    DestroyGLFW();
}