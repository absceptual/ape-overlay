
#include "window/window.h"
#include "renderer/renderer.h"
#include "util/process.h"

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count)
{
	// Overlay initalization and attachment
	auto overlay = window(L"EpicGamesLauncher.exe", instance);


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
			 doggy->draw_line({ 0, 0 }, { 500, 500 }, { 0, 255, 0 });
			// doggy->draw_line({ 300, 100 }, { 400, 200 }, { 255, 0, 0 }, 3.0f);

			// doggy->draw_box({ 500, 500 }, 50, 50, { 0, 255, 0 }, 2.0f);
			doggy->end();
		}
	}

	delete doggy;
	return 0;
}