#include "window.h"


int window::width = 0;
int window::height = 0;
window::pixel window::position = { 0 };

window::window(const wchar_t* process, HINSTANCE instance)
{

	WNDCLASS wndclass{ };
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	// wndclass.lpfnWndProc = window::procedure;
	wndclass.lpszClassName = L"ape!";
	wndclass.hInstance = instance;
	RegisterClass(&wndclass);

	if (!attach(process))
		throw std::runtime_error("Failed to find target process window!");

	// Gets the target window size and position
	RECT area{ };
	GetClientRect(m_target, &area);
	
	if (!area.left && !area.right && !area.bottom && !area.top)
	{
		// Attempt to unminimize window (assuming its minimized) and try again
		ShowWindow(m_target, SW_SHOWNORMAL);
		GetClientRect(m_target, &area);

		if (!area.left && !area.right && !area.bottom && !area.top)
			throw std::runtime_error("Could not determine window size! (is it minimized?)");
	}

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

		m_position = { window_area.left + (diff.x - window_area.left), window_area.top + (diff.y - window_area.top), };
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


// Hijacks NVIDIA Share overlay in order to draw over our target process
window::window(const wchar_t* process)
{
	// Overlay has a class named CEF-OSC-WIDGET with the attributes we need
	m_handle = FindWindow(L"CEF-OSC-WIDGET", L"NVIDIA GeForce Overlay");
	if (!m_handle)
		throw std::runtime_error("Failed to find NVIDIA Share overlay (is it running?)");

	if (!attach(process) || m_target == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Failed to find target window! (is it running?)");

	// Gets the target window size and position
	RECT area{ };
	GetClientRect(m_target, &area);
	{
		if (!area.left && !area.right && !area.bottom && !area.top)
		{
			// Attempt to unminimize window (assuming its minimized) and try again
			ShowWindow(m_target, SW_SHOWNORMAL);
			GetClientRect(m_target, &area);

			if (!area.left && !area.right && !area.bottom && !area.top)
				throw std::runtime_error("Could not determine window size! (is it minimized?)");
		}
	}

	
	{	
		POINT position{ };
		MapWindowPoints(m_target, HWND_DESKTOP, &position, 1);

		m_width = area.right - area.left;
		m_height = area.bottom - area.top;

		RECT client_area{ };
		GetClientRect(m_handle, &client_area);

		RECT window_area{ };
		GetClientRect(m_handle, &window_area);

		POINT diff{ };
		ClientToScreen(m_handle, &diff);

		m_position = { window_area.left + (diff.x - window_area.left), window_area.top + (diff.y - window_area.top), };
		WIDTH_OFFSET = position.x;
		HEIGHT_OFFSET = position.y;

		// MSDN states that negative margin values create a "sheet of glass" effect (no window border/solid surface)
		SetLayeredWindowAttributes(m_handle, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

		MARGINS margins = { -1 };
		DwmExtendFrameIntoClientArea(m_handle, &margins);

	}
}

std::pair<int, HWND> window::get_z_order(HWND hwnd)
{
	int z = 0;
	HWND target = NULL;
	for (target = GetTopWindow(NULL); target != hwnd; target = GetNextWindow(target, GW_HWNDNEXT))
		++z;

	return std::make_pair(z, target);
}

// TODO: Setup proper window manager
window::handler_t window::hk_procedure(int code, WPARAM wparam, LPARAM lparam)
{
	auto proc_data = reinterpret_cast<CWPSTRUCT*>(lparam);

	auto msg = proc_data->message;
	switch (msg)
	{
	case WM_WINDOWPOSCHANGING:
		{
			auto data = reinterpret_cast<WINDOWPOS*>(proc_data->lParam);
			window::width = data->cx;
			window::height = data->cy;
			window::position = { data->x, data->y };
			// window::top_window = data->hwndInsertAfter;
		}
	}
	return CallNextHookEx(NULL, code, wparam, lparam);
}

/*
bool window::handler(window* window, MSG& message)
{
	if (PeekMessage(&message, window->get_target(), NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
	return !(message.message == WM_QUIT);
}
*/

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
