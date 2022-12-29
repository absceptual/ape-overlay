// Hijacks NVIDIA Share overlay (signed application) in order to draw over our preferred game

#pragma once
#include <stdexcept>
#include <utility>

#include <Windows.h>
#include <dwmapi.h>
#include <d2d1.h>

#include "../../util/process.h"

#pragma comment(lib, "Dwmapi.lib")
#pragma warning(disable : 6387)

namespace overlay
{
	namespace window
	{
		struct pixel
		{
			int x, y;
		};
		using size_t = pixel;

		inline size_t		  size{ };
		inline pixel          position{ };
		inline HWND           handle{ };
		inline HWND           target{ };
	}
	
	auto init( const wchar_t* name ) -> void;
	auto attach( const wchar_t* name ) -> bool;
}
