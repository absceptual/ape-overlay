#include "window.h"

// TODO: Make the overlay transparent and research what flags cause detections
window::window(const int width, const int height, const wchar_t* title) : m_width{ width }, m_height{ height }
{
	WNDCLASS wndclass{ };
	wndclass.style = CS_OWNDC;
	wndclass.lpfnWndProc = window::handler;
	wndclass.hCursor = NULL;
	wndclass.lpszClassName = L"ape!overlay";
	RegisterClass(&wndclass);

	m_handle = CreateWindowEx(
		0,
		wndclass.lpszClassName,
		title,
		WS_CAPTION | WS_POPUP | WS_SYSMENU | WS_VISIBLE,
		100,
		100,
		width,
		height,
		NULL,
		NULL,
		NULL,
		NULL
	);

	if (m_handle == INVALID_HANDLE_VALUE)
	{
		throw std::runtime_error("Failed to create window");
		m_initalized = false;
	}

	m_initalized = true;
}

// TODO: Setup proper window manager
window::handler_t window::handler(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	return DefWindowProc(handle, message, wparam, lparam);
}

HWND window::get_hwnd() { return m_handle; }
bool window::is_initalized() { return m_initalized; }