#include "window.h"

// Hijacks NVIDIA Share overlay in order to draw over our target process
window::window(const wchar_t* process)
{
	// Overlay has a class named CEF-OSC-WIDGET with the attributes we need
	m_handle = FindWindow(L"CEF-OSC-WIDGET", L"NVIDIA GeForce Overlay");
	if (!m_handle)
		throw std::runtime_error("Failed to find NVIDIA Share overlay (is it running?)");

	if (!attach(process) || m_target == INVALID_HANDLE_VALUE)
		throw std::runtime_error("Failed to find target window! (is it running?)");


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

	// Define overlay boundaries (for our renderer to detect changes in screen resolution)
	GetWindowRect(m_handle, &area);
	window::width  = area.right - area.left;
	window::height = area.bottom - area.top;
	window::offset = { position.x, position.y };


	auto style = GetWindowLongPtr(m_handle, GWL_EXSTYLE);
	SetWindowLongPtr(m_handle, GWL_EXSTYLE, style | (WS_EX_TRANSPARENT | WS_EX_LAYERED));
	auto f = GetClassLongPtr(m_handle, GCL_STYLE);
	SetLayeredWindowAttributes(m_handle, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

	// MSDN states that negative margin values create a "sheet of glass" effect (no window border/solid surface)
	MARGINS margins = { -1, -1, -1, -1};
	DwmExtendFrameIntoClientArea(m_handle, &margins);

	ShowWindow(m_handle, SW_NORMAL);
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

int window::width = 0;
int window::height = 0;
window::pixel window::offset = { 0, 0 };