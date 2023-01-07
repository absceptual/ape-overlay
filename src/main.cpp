
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

		D2D1_POINT_2F center{ overlay::window::t_size.x / 2,  overlay::window::t_size.y / 2 };

		renderer::draw_line( { center.x, center.y - 25 }, { center.x, center.y + 25 }, D2D1::ColorF( 0.f, 0.f, 0.f, 1.0f ), 3.f );
		renderer::draw_line( { center.x - 25, center.y }, { center.x + 25, center.y }, D2D1::ColorF( 0.f, 0.f, 0.f, 1.0f ), 3.f);
		// renderer::draw_filled_box( center, { 50, 50 }, D2D1::ColorF( 0.f, 0.f, 0.f, 1.0f ));
		renderer::draw_box( {0, 0 }, { 50, 50 }, D2D1::ColorF( 0.f, 0.f, 0.f, 1.0f ), false, 5.0f);
		// renderer::draw_text( {0, 0 }, L"pussy balls!", D2D1::ColorF( 0.f, 0.f, 0.f, 1.0f ) );
		
		renderer::end( );
	}
	return 0;
}