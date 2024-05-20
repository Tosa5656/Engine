#include <iostream>
#include <Core.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
	Window window = Window(nCmdShow, L"Engine", L"Engine", 0, 0, 800, 600);
	system("pause");
}
