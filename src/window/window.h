#pragma once
#include <stdexcept>
#include <utility>

#include <Windows.h>
#include <dwmapi.h>

#include "../util/process.h"

#pragma warning(disable : 6387)

// Hijacks NVIDIA Share overlay (signed application) in order to draw over our preferred game
class window
{
public:
	struct pixel
	{
		int x, y;
	};

	static int width;
	static int height;
	static pixel offset;

private:
	using handler_t		     = LRESULT;
	HWND m_handle           { };
	HWND m_target           { };

public:
	window(const wchar_t* process);
	window() = default;

	// Iterates through all open windows and finds one that matches the target process (using process identifiers)
	bool attach(const wchar_t* process);

	auto& get_hwnd()           { return m_handle; }
	auto& get_target()         { return m_target; }
};

