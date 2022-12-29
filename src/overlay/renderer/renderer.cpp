#include "renderer.h"

// Add text and image rendering support
auto renderer::init( const wchar_t* name, const wchar_t* font ) -> bool
{
	try { 
		overlay::init( name );
	}
	catch (std::exception exception ) {
		MessageBoxA( NULL, exception.what( ), NULL, MB_ABORTRETRYIGNORE );
		throw exception;
	}

	DXGI_SWAP_CHAIN_DESC chain{  };
	chain.BufferCount = 1;
	chain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chain.OutputWindow = overlay::window::handle;
	chain.SampleDesc.Count = 1;
	chain.Windowed = true;

	D3D_FEATURE_LEVEL levels[]
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	auto status = D3D11CreateDeviceAndSwapChain( NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_BGRA_SUPPORT, levels, ARRAYSIZE( levels ), D3D11_SDK_VERSION, &chain, &objects::swapchain, &objects::d_device, NULL, NULL);
	if ( FAILED( status ) )
		return false;

	status = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof( ID2D1Factory ), reinterpret_cast< void** >( &objects::factory ));
	if ( FAILED( status ) )
		return false;

	status = DWriteCreateFactory( DWRITE_FACTORY_TYPE_SHARED, __uuidof( IDWriteFactory ), reinterpret_cast< IUnknown** >( &font::factory ) );
	if ( FAILED( status ) )
		return false;

	status = font::factory->CreateTextFormat( font, NULL, font::weight, font::style, font::stretch, font::size, L"", &font::format );
	if ( FAILED( status ) )
		return false;

	font::format->SetTextAlignment( DWRITE_TEXT_ALIGNMENT_CENTER );
	font::format->SetParagraphAlignment( DWRITE_PARAGRAPH_ALIGNMENT_CENTER );
	font::name = name;

	status = objects::d_device->QueryInterface( __uuidof( IDXGIDevice ), ( void** )( &objects::dxgi_device ) );
	if ( FAILED( status ) )
		return false;

	status = objects::factory->CreateDevice( objects::dxgi_device, &objects::device );
	if ( FAILED( status ) )
		return false;

	status = objects::device->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &objects::ctx );
	if ( FAILED( status ) )
		return false;

	status = objects::swapchain->GetBuffer( 0, IID_PPV_ARGS( &objects::backbuffer ) );
	if ( FAILED( status ) )
		return false;

	D2D1_BITMAP_PROPERTIES1 properties{ D2D1::PixelFormat( DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED ), 0, 0, D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW , nullptr };
	status = objects::ctx->CreateBitmapFromDxgiSurface( objects::backbuffer, &properties, &objects::bitmap );
	if ( FAILED( status ) )
		return false;

	status = objects::ctx->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::Black ), &objects::brush );
	if ( FAILED( status ) )
		return false;

	return true;
}

void renderer::begin( )
{
	objects::ctx->BeginDraw( );
	objects::ctx->SetTarget( objects::bitmap );
	objects::ctx->Clear( D2D1::ColorF( 0.f, 0.f, 0.f, 0.f ) );

	renderer::update( );
}

void renderer::update( )
{
	RECT area{ };
	GetClientRect( overlay::window::target, &area );
	overlay::window::t_size = { area.right - area.left, area.bottom - area.top };

	POINT point{ };
	MapWindowPoints( overlay::window::target, HWND_DESKTOP, &point, 1 );
	overlay::window::position = { point.x, point.y };

	SetWindowPos( overlay::window::target, overlay::window::handle, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE );
	if ( !IsWindowVisible( overlay::window::handle ) )
		ShowWindow( overlay::window::handle, SW_NORMAL );

	GetWindowRect( overlay::window::handle, &area );
	auto width  = area.right - area.left;
	auto height = area.bottom - area.top;
	if ( overlay::window::size.x != width || overlay::window::size.y != height )
	{
		// Release all resources, re-initalize the renderer and begin drawing again
		renderer::reset( );
		renderer::init( overlay::window::process );
		renderer::begin( );
	}
}

void renderer::end( )
{
	auto status = objects::ctx->EndDraw( );
	objects::swapchain->Present( 0, 0 );
}

void renderer::reset( )
{
	safe_release( objects::factory );
	safe_release( objects::device );
	safe_release( objects::ctx );
	safe_release( objects::brush );
	safe_release( objects::target );
	safe_release( objects::bitmap );
	safe_release( objects::d_device );
	safe_release( objects::dxgi_device );
	safe_release( objects::swapchain );
	safe_release( objects::backbuffer );
}

void renderer::draw_line( D2D1_POINT_2F start, D2D1_POINT_2F end, D2D1_COLOR_F color, float thickness )
{
	start.x += overlay::window::position.x;
	end.x += overlay::window::position.x;

	start.y += overlay::window::position.y;
	end.y += overlay::window::position.y;

	objects::brush->SetColor( color );
	objects::ctx->DrawLine( start, end, objects::brush, thickness );
}


auto renderer::draw_box( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float centered, float thickness ) -> D2D_RECT_F
{
	D2D_RECT_F dimensions = { };
	position.x += overlay::window::position.x;
	position.y += overlay::window::position.y;
	objects::brush->SetColor( color );

	// We want to center our drawing instead of starting from the top left
	if ( centered )
	{
		auto offset_x = size.width / 2;
		auto offset_y = size.height / 2;
		dimensions = { position.x - offset_x, position.y - offset_y, position.x + offset_x, position.y + offset_y };
		objects::ctx->DrawRectangle( dimensions, objects::brush, thickness );
	}
	else
	{
		dimensions = { position.x, position.y, position.x + size.width, position.y + size.height };
		objects::ctx->DrawRectangle( dimensions, objects::brush, thickness );
	}
	return dimensions;
}

auto renderer::draw_circle( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float thickness ) -> D2D1_ELLIPSE
{
	D2D1_ELLIPSE ellipse = { position, size.width, size.height };
	objects::brush->SetColor( color );
	objects::ctx->DrawEllipse( ellipse, objects::brush, thickness );

	return ellipse;
}

void renderer::draw_filled_box( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color, float centered)
{
	auto dimensions = renderer::draw_box( position, size, color, centered );
	objects::ctx->FillRectangle( dimensions, objects::brush );
}

void renderer::draw_filled_circle( D2D1_POINT_2F position, D2D_SIZE_U size, D2D1_COLOR_F color)
{
	auto ellipse = renderer::draw_circle( position, size, color );
	objects::ctx->FillEllipse( ellipse, objects::brush );
}

void renderer::draw_text( D2D1_POINT_2F position, const wchar_t* text, D2D1_COLOR_F color )
{
	position.x += overlay::window::position.x;
	position.y += overlay::window::position.y;

	D2D1_SIZE_F render_size = { overlay::window::size.x, overlay::window::size.y };
	D2D1_RECT_F dimensions = { position.x , position.y, render_size.width, render_size.height };
	objects::brush->SetColor( color );
	objects::ctx->DrawText( text, wcslen( text ) - 1, font::format, dimensions, objects::brush );
}