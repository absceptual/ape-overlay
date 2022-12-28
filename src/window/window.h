#pragma once

#include <stdexcept>

#include <Windows.h>
#include <dwmapi.h>

#include "../util/process.h"

// Change as needed
constexpr int HEIGHT_OFFSET = 10;
constexpr int WIDTH_OFFSET = 6;

class window
{
private:
	using handler_t = LRESULT;
	HWND m_handle{ };
	HWND m_target{ };

	int m_width{ };
	int m_height{ };

	// Iterates through all open windows and finds one that matches the target process (using process identifiers)
	bool attach(const wchar_t* process);
public:
	window(const wchar_t* process, HINSTANCE instance);
	window() = default;

	static handler_t procedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	static bool handler(window& window, MSG& message);
	
	HWND get_hwnd();
	float get_width() { return m_width; }
	float get_height() { return m_height; }

	int m_drawing_xoffset = 0;
	int m_drawing_yoffset = 0;
public:

};
