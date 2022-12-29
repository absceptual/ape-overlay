
#include "overlay/window/window.h"
#include "overlay/renderer/renderer.h"
#include "util/process.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dwmapi.lib")

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count)
{
	// Overlay initalization and attachment
	try {
		renderer::init( L"notepad.exe" );
	}
	catch (std::runtime_error exception) {
		MessageBoxA(NULL, exception.what(), NULL, MB_ABORTRETRYIGNORE);
		return -1;
	}

	while (true)
	{
		renderer::begin( );

		auto x_offset = overlay::window::position.x;
		auto y_offset = overlay::window::position.y;
		renderer::objects::ctx->DrawRectangle(D2D1::RectF( x_offset + 100.0f, y_offset + 100.0f, x_offset + 300.f, y_offset + 300.f ), renderer::objects::brush);
		
		renderer::end( );
	}
	return 0;
}