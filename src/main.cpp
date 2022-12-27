#include <iostream>

#include "window/window.h"
#include "renderer/renderer.h"

auto WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count) -> int
{
	// Initalize our overlay, setup D3D11 and then process extra data
	auto overlay = window(800, 600, L"balls!");
	renderer* doggy = nullptr;
	try
	{
		doggy = new renderer(overlay);
	}
	catch (std::runtime_error e)
	{
		std::cout << e.what();
	}

	MSG message{ };
	while (true)
	{
		if (PeekMessage(&message, overlay.get_hwnd(), NULL, NULL, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);

			if (message.message == WM_QUIT)
				break;
		}

		doggy->update(overlay);
		if (doggy->begin())
		{
			// fix drawing
			doggy->draw_line({ 0, 0 }, { 800, 0 }, { 255, 0, 0 }, 5.0f);
			doggy->draw_box({ 250, 250 }, 200, 200, { 0, 255, 0 }, 3.0f);
			doggy->end();
		}
	}

	delete doggy;
	return 0;
}