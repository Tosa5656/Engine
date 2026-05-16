#include <window/window.h>

void Awake() {}
void Start() {}
void Update() {}

int main()
{
    Window window1("Singularity Engine", 1280, 720, Awake, Start, Update);

    while (window1.ShouldActive())
    {
        if (window1.ShouldActive()) window1.Update();
    }

}
