
#include "window/window.h"
#include "renderer/renderer.h"

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count)
{
	// Overlay initalization and attachment
	auto overlay = window(800, 600, L"balls!", instance);


	renderer* doggy = nullptr;
	try
	{
		doggy = new renderer(overlay);
	}
	catch (std::runtime_error exception)
	{
		
	}

	MSG message{ };
	while (true)
	{
		

		doggy->update(overlay);
		if (doggy->begin())
		{
			doggy->draw_line({ 0, 0 }, { 800, 0 }, { 255, 0, 0 }, 5.0f);
			doggy->draw_box({ 250, 250 }, 200, 200, { 0, 255, 0 }, 3.0f);
			doggy->end();
		}
	}

	delete doggy;
	return 0;
}