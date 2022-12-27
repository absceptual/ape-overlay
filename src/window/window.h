#pragma once

#include <stdexcept>
#include <Windows.h>

/*
TODO:
	- Make the overlay *actually* an overlay
	- fix wndproc handler on wm_quit and other shit
*/
class window
{
private:
	using handler_t = LRESULT;
	HWND m_handle{ };
	
	const int m_width{ };
	const int m_height{ };
	bool m_initalized{ };

public:
	window(const int width, const int height, const wchar_t* title);
	static handler_t handler(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	bool is_initalized();
	HWND get_hwnd();
};
