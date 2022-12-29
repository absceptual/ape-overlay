#pragma once


#include <concepts>
#include <chrono>
#include <fstream>
#include <vector>
#include <memory>

#include <Windows.h>

#include <d2d1.h>
#include <d2d1_1.h>
#include <DirectXHelpers.h>
#include <D2d1_1helper.h>
#include <dwrite.h>

#include "../window/window.h"

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite.lib")

namespace renderer
{
	namespace objects
	{
		inline ID2D1Factory1* factory			= nullptr;
		inline ID2D1Device* device				= nullptr;
		inline ID2D1DeviceContext* ctx			= nullptr;
		inline ID2D1SolidColorBrush* brush		= nullptr;
		inline ID2D1HwndRenderTarget* target	= nullptr;
		inline ID2D1Bitmap1* bitmap				= nullptr;
		inline ID3D11Device* d_device			= nullptr;
		inline IDXGIDevice* dxgi_device			= nullptr;
		inline IDXGISwapChain* swapchain		= nullptr;
		inline IDXGISurface* backbuffer			= nullptr;
	}

	namespace font
	{
		inline IDWriteFactory* factory		= nullptr;
		inline IDWriteTextFormat* format	= nullptr;

		inline DWRITE_FONT_WEIGHT weight	= DWRITE_FONT_WEIGHT_NORMAL;
		inline DWRITE_FONT_STYLE style		= DWRITE_FONT_STYLE_NORMAL;
		inline DWRITE_FONT_STRETCH stretch  = DWRITE_FONT_STRETCH_NORMAL;
		inline FLOAT size					= 50;
		inline const wchar_t* name = L"Verdana";
	}

	auto init( const wchar_t* name, const wchar_t* font = font::name ) -> bool;
	
	template <class T> void safe_release( T object )
	{
		if ( std::is_base_of<IUnknown, T>::value && std::is_pointer<T>::value && object )
			object->Release( );
	}

	void begin( );
	void update( );
	void end( );
	void reset( );

	// All drawings are offset to match the client rectangle of the target window
	void draw_line( D2D1_POINT_2F start, D2D1_POINT_2F end, D2D1_COLOR_F color, float thickness = 1.0f);
	auto draw_box( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float centered = false, float thickness = 1.0f ) -> D2D_RECT_F;
	auto draw_circle( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float thickness = 1.0f ) -> D2D1_ELLIPSE;
	void draw_filled_box( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float centered = false );
	void draw_filled_circle( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color );
	void draw_text( D2D1_POINT_2F position, const wchar_t* text, D2D1_COLOR_F color );
};

