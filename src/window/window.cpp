#include "window.h"


// TODO: Make the overlay transparent and research what flags cause detections
window::window(const int width, const int height, const wchar_t* title, HINSTANCE instance) : m_width{ width }, m_height{ height }
{
	WNDCLASS wndclass{ };
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = window::procedure;
	wndclass.lpszClassName = L"ape!overlay";
	wndclass.hInstance = instance;
	RegisterClass(&wndclass);

	// Find an alternative to TOPMOST/TRANSPARENT/LAYERED
	// Initally creates a window larger than 1920x1080 (to get past anticheats checking for exact overlay sizes)
	m_handle = CreateWindowEx(
		WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
		wndclass.lpszClassName,
		title,
		 WS_POPUP | WS_VISIBLE,
		0 - WIDTH_OFFSET, 
		0 + HEIGHT_OFFSET,
		WINDOW_WIDTH + WIDTH_OFFSET,
		WINDOW_HEIGHT + HEIGHT_OFFSET,
		NULL,
		NULL,
		instance,
		NULL
	);

	if (m_handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Failed to create window");

	// Used to make our overlay visible
	SetLayeredWindowAttributes(m_handle, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);
	{
		RECT client_area{ };
		GetClientRect(m_handle, &client_area);

		RECT window_area{ };
		GetClientRect(m_handle, &window_area);

		POINT diff{ };
		ClientToScreen(m_handle, &diff);

		const MARGINS margins
		{
			window_area.left + (diff.x - window_area.left),
			window_area.top + (diff.y - window_area.top),
			client_area.right,
			client_area.bottom
		};

		DwmExtendFrameIntoClientArea(m_handle, &margins);
	}
}

// TODO: Setup proper window manager
window::handler_t window::procedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam)
{
	switch (message)
	{
	case WM_DESTROY:
	case WM_CLOSE:
		PostQuitMessage(0x0);
		return 0;

	default:
		return DefWindowProc(handle, message, wparam, lparam);
	}
}

bool window::handler(window& window, MSG& message)
{
	if (PeekMessage(&message, window.get_hwnd(), NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return message.message == WM_QUIT;
}

bool window::attach(const wchar_t* process)
{
	std::uint32_t target_pid = process::get_id(process);
	if (!target_pid)
		return false;

	auto enumerator = [this](HWND hwnd, LPARAM pid) -> BOOL {
		DWORD window_pid{ };
		GetWindowThreadProcessId(hwnd, &window_pid);

		if (window_pid != pid)
			return TRUE;

		m_target = hwnd;
		return FALSE;
	};

	return EnumWindows(reinterpret_cast<WNDENUMPROC>(&enumerator), target_pid);
}


HWND window::get_hwnd() 
{ 
	return m_handle; 
}
