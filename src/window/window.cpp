#include "window.h"


// TODO: Research what flags cause detections
window::window(const wchar_t* process, HINSTANCE instance)
{
	WNDCLASS wndclass{ };
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = window::procedure;
	wndclass.lpszClassName = L"ape!overlay";
	wndclass.hInstance = instance;
	RegisterClass(&wndclass);

	if (!attach(process))
		throw std::runtime_error("Failed to find target process window!");

	// Gets the target window size and position
	RECT area{ };
	GetClientRect(m_target, &area);
	
	if (!area.left && !area.right && !area.bottom && !area.top)
		throw std::runtime_error("Could not determine window size! (is it minimized?)");

	POINT position{ };
	MapWindowPoints(m_target, HWND_DESKTOP, &position, 1);
	{
		this->m_width = area.right - area.left;
		this->m_height = area.bottom - area.top;

		int window_width = m_width + WIDTH_OFFSET;
		int window_height = m_height + HEIGHT_OFFSET;

		m_handle = CreateWindowEx(
			WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
			wndclass.lpszClassName,
			L"ape!",
			WS_POPUP | WS_VISIBLE,
			position.x - WIDTH_OFFSET,
			position.y - HEIGHT_OFFSET,
			window_width,
			window_height,
			NULL,
			NULL,
			instance,
			NULL
		);

		if (m_handle == INVALID_HANDLE_VALUE)
			throw std::runtime_error("Failed to create window");
	}
	
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

	struct lambda_info
	{
		void* _this;
		DWORD target_pid;
	};

	lambda_info info{ this, target_pid };

	return !EnumWindows([](HWND hwnd, LPARAM info) -> BOOL {
		DWORD window_pid{ };
		lambda_info* linfo = reinterpret_cast<lambda_info*>(info);

		GetWindowThreadProcessId(hwnd, &window_pid);

		if (window_pid != linfo->target_pid)
			return TRUE;

		reinterpret_cast<window*>(linfo->_this)->m_target = hwnd;
		return FALSE;
	}, reinterpret_cast<LPARAM>(&info));
}

