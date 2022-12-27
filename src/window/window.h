#pragma once

#include <stdexcept>

#include <Windows.h>
#include <dwmapi.h>

constexpr int WINDOW_HEIGHT = 1930; // 1920 = 1930 - 10
constexpr int WINDOW_WIDTH = 1086; // 1080  = 1086 - 6
constexpr int HEIGHT_OFFSET = 10;
constexpr int WIDTH_OFFSET = 6;

class window
{
private:
	using handler_t = LRESULT;
	HWND m_handle{ };
	
	const int m_width{ };
	const int m_height{ };

public:
	window(const int width, const int height, const wchar_t* title, HINSTANCE instance);

	static handler_t procedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	static bool handler(window& window, MSG& message);

	bool attach(const wchar_t* process);
	HWND get_hwnd();
};
