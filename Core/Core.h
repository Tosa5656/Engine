#pragma once

#include <iostream>
#include "../Graphics/Window.h"
#include "../Graphics/Graphics.h"
#include "Debug.h"

class EngineEditor
{
public:
	EngineEditor()
	{

	}
	
	void Start()
	{
		Window window = Window(m_nCmdShow, L"Engine", L"Engine", 0, 0, 800, 600);
		m_isInited = true;
		Debug::Log("Engine started");
	}
	void Stop()
	{
		m_isInited = false;
		Debug::Log("Engine shutdown");
	}
private:
	bool m_isInited = false;
	HINSTANCE m_hInstance;
	int m_nCmdShow;
};