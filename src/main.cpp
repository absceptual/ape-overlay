
#include "window/window.h"
#include "renderer/renderer.h"
#include "util/process.h"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "Dwmapi.lib")

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count)
{
	// Overlay initalization and attachment
	std::unique_ptr<renderer> render;

	try {
		render = std::make_unique<renderer>(L"notepad.exe");
	}
	catch (std::runtime_error exception) {
		MessageBoxA(NULL, exception.what(), NULL, MB_ABORTRETRYIGNORE);
		return -1;
	}

	MSG message{ };
	while (true)
	{
		renderer::update(render);
		render->begin();

		render->draw_filled_box({ 0, 0 }, 50, 50, { 255, 0, 0 }, 3.0f);
		
		render->end();
	}
	return 0;
}