#include <window/window.h>

void Awake() {}
void Start() {}
void Update() {}

int main()
{
    Window window1("Singularity Engine - Main", 1280, 720, Awake, Start, Update);
    Window window2("Singularity Engine - Secondary", 640, 480, Awake, Start, Update, &window1);

    while (window1.ShouldActive() || window2.ShouldActive())
    {
        if (window1.ShouldActive()) window1.Update();
        if (window2.ShouldActive()) window2.Update();
    }

}
