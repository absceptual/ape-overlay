
#include "window/window.h"
#include "renderer/renderer.h"
#include "util/process.h"

int APIENTRY WinMain(HINSTANCE instance, HINSTANCE prev, LPSTR cmd, int count)
{
	// Overlay initalization and attachment
	std::unique_ptr<renderer> render;
	try {
		render = std::make_unique<renderer>(L"notepad.exe", instance);
	}
	catch (std::runtime_error exception) {
		MessageBoxA(NULL, exception.what(), NULL, MB_ABORTRETRYIGNORE);
		return -1;
	}

	MSG message{ };
	while (true)
	{
		render->update();
		render->begin();

		render->draw_filled_box({ 0, 0 }, 50, 50, { 255, 0, 0 }, 3.0f);
		render->draw_box({ 55, 0 }, 50, 50, { 255, 0, 0 }, 3.0f);
		render->draw_line({ 150, 0 }, {250, 0}, {255, 0, 0}, 3.0f);

		render->end();
	}
	return 0;
}