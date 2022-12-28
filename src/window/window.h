#pragma once
#include <stdexcept>

#include <Windows.h>
#include <dwmapi.h>

#include "../util/process.h"

const int WIDTH_OFFSET = 5;
const int HEIGHT_OFFSET = 6;
// Hijacks NVIDIA Share overlay (signed application) in order to draw over our preferred game
class window
{
public:
	struct pixel
	{
		int x, y;
	};

private:
	using handler_t		     = LRESULT;
	HWND m_handle           { };
	HWND m_target           { };

	int m_width             { };
	int m_height            { };
	int z_order				{ };
	pixel m_position		{ };	

public:
	window(const wchar_t* process);
	window(const wchar_t* process, HINSTANCE instance);
	window() = default;

	// Iterates through all open windows and finds one that matches the target process (using process identifiers)
	bool attach(const wchar_t* process);

	// Window handler
	static handler_t procedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	// Sends windows messages to window::procedure
	static bool handler(window& window, MSG& message);
	
	void update_z();

	// Getters...
	auto& get_hwnd()           { return m_handle; }
	auto& get_target()         { return m_target; }
	auto& get_width()          { return m_width; }
	auto& get_height()		  { return m_height; }
	auto& get_position()		  { return m_position; }
};
