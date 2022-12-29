#include "window.h"

auto overlay::init( const wchar_t* process ) -> void
{
	window::handle = FindWindow(L"CEF-OSC-WIDGET", L"NVIDIA GeForce Overlay");
	if (!window::handle)
		throw std::runtime_error( "Failed to find NVIDIA Share overlay (is it running?)" );

	if (!attach( process ) || window::handle == INVALID_HANDLE_VALUE)
		throw std::runtime_error( "Failed to find target window! (is it running?)" );

	RECT area{ };
	GetClientRect( window::target, &area );
	if (!area.left && !area.right && !area.bottom && !area.top)
	{
		// Attempt to unminimize window (assuming its minimized) and try again
		ShowWindow( window::target, SW_SHOWNORMAL );
		GetClientRect( window::target, &area);

		if (!area.left && !area.right && !area.bottom && !area.top)
			throw std::runtime_error( "Could not determine window size! (is it minimized?)" );
	}
	
	POINT position{ };
	MapWindowPoints( window::target, HWND_DESKTOP, &position, 1 );
	GetWindowRect( window::handle, &area );

	// Define overlay boundaries (for our renderer to detect changes in screen resolution)
	window::size     = { area.right - area.left, area.bottom - area.top };
	window::position = { position.x, position.y };

	auto style = GetWindowLongPtr( window::handle, GWL_EXSTYLE );
	SetWindowLongPtr( window::handle, GWL_EXSTYLE, style | (WS_EX_TRANSPARENT | WS_EX_LAYERED) );
	SetLayeredWindowAttributes( window::handle, RGB(0, 0, 0), BYTE(255), LWA_ALPHA );

	// MSDN states that negative margin values create a "sheet of glass" effect (no window border/solid surface)
	MARGINS margins = { -1, -1, -1, -1 };
	DwmExtendFrameIntoClientArea( window::handle, &margins );
	ShowWindow( window::handle, SW_NORMAL );
}
  
bool overlay::attach( const wchar_t* name )
{
	std::uint32_t pid = process::get_id( name );
	if (!pid )
		return false;

	return !EnumWindows( []( HWND hwnd, LPARAM info ) -> BOOL {
		DWORD id{ 0 };
		GetWindowThreadProcessId( hwnd, &id );

		if ( id != static_cast<DWORD>(info))
			return TRUE;

		window::target = hwnd;
		return FALSE;
	}, pid );
}
