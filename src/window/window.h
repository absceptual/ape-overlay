#pragma once
#include <stdexcept>

#include <Windows.h>
#include <dwmapi.h>

#include "../util/process.h"

// Change as needed
constexpr int HEIGHT_OFFSET = 10;
constexpr int WIDTH_OFFSET = 6;

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
	pixel m_position		{ };	

public:
	/* 
	Responsible for dynamically finding the target window based off executable name / pid and creating an overlay
	The overlay is slightly bigger than the target window (HEIGHT/WIDTH offset) to throw off overlay checks by anticheats
	*/
	window(const wchar_t* process, HINSTANCE instance);
	window() = default;

	// Iterates through all open windows and finds one that matches the target process (using process identifiers)
	bool attach(const wchar_t* process);

	// Window handler
	static handler_t procedure(HWND handle, UINT message, WPARAM wparam, LPARAM lparam);

	// Sends windows messages to window::procedure
	static bool handler(window& window, MSG& message);
	
	// Getters...
	auto& get_hwnd()           { return m_handle; }
	auto& get_target()         { return m_target; }
	auto& get_width()          { return m_width; }
	auto& get_height()		  { return m_height; }
	auto& get_position()		  { return m_position; }
};
