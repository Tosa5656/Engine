#pragma once
#include <iostream>
#include <tchar.h>
#include <string>
#include <Windows.h>
#include "Vector.h"

#define DEFAULT_CURSOR IDC_ARROW

//Message boxes
class MessageBoxType {
public:
	MessageBoxType(UINT Buttons, UINT Icon)
	{
		buttons = Buttons;
		icon = Icon;
	}

	UINT GetButtons()
	{
		return buttons;
	}

	UINT GetIcon()
	{
		return icon;
	}
private:
	UINT buttons;
	UINT icon;
};

int SendMessageBox(MessageBoxType type, const wchar_t* Text, const wchar_t* Caption)
{
	int MsgBoxId = MessageBox(NULL, Text, Caption, type.GetButtons() | type.GetIcon());
	return MsgBoxId;
}

//Windows
struct Window
{
	bool m_isCreated = false;

	HWND m_hwnd;
	HINSTANCE m_hInstance;
	LPCWSTR m_Title;
	LPCWSTR m_Class;
	Vector2 m_Pos;
	Vector2 m_Size;
	int m_nCmdShow;

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
			break;
		}
	}

	Window(int m_nCmdShow, const wchar_t* Title, const wchar_t* Class, Vector2 Pos, Vector2 Size)
	{
		m_hInstance = GetModuleHandle(NULL);
		m_Title = LPCWSTR(Title);
		m_Class = LPCWSTR(Class);
		m_Pos = Pos;
		m_Size = Size;

		Create();
	}

	Window(int m_nCmdShow, const wchar_t* Title, const wchar_t* Class, int x, int y, int Width, int Height)
	{
		m_hInstance = GetModuleHandle(NULL);
		m_Title = LPCWSTR(Title);
		m_Class = LPCWSTR(Class);
		m_Pos = Vector2(x, y);
		m_Size = Vector2(Width, Height);

		Create();
	}

	bool Create()
	{
		HCURSOR cursor = LoadCursor(NULL, DEFAULT_CURSOR);

		WNDCLASSEX wcex;

		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = m_hInstance;
		wcex.hIcon = LoadIcon(wcex.hInstance, IDI_APPLICATION);
		wcex.hCursor = cursor;
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = m_Class;
		wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

		if (!RegisterClassEx(&wcex))
		{
			SendMessageBox(MessageBoxType(MB_OK, MB_ICONERROR), _T("Call to RegisterClassEx failed!"), m_Title);
			m_isCreated = false;
			return m_isCreated;
		}

		m_hwnd = CreateWindowEx(
			WS_EX_OVERLAPPEDWINDOW,
			m_Class,
			m_Title,
			WS_OVERLAPPEDWINDOW,
			m_Pos.x, m_Pos.y,
			m_Size.x, m_Size.y,
			NULL,
			NULL,
			m_hInstance,
			NULL
		);

		if (!m_hwnd)
		{
			SendMessageBox(MessageBoxType(MB_OK, MB_ICONERROR), _T("Call to CreateWindow failed!"), m_Title);
			m_isCreated = false;
			return m_isCreated;
		}

		ShowWindow(m_hwnd, m_nCmdShow);
		UpdateWindow(m_hwnd);

		MSG msg;
		ZeroMemory(&msg, sizeof(MSG));
		while (true)
		{
			BOOL PeekMessageL(
				LPMSG lpMsg,
				HWND hWnd,
				UINT wMsgFilterMin,
				UINT wMsgFilterMax,
				UINT wRemoveMsg
			);

			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		m_isCreated = true;
		return m_isCreated;
	}
};