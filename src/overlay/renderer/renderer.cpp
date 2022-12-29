#include "renderer.h"

auto renderer::init( const wchar_t* name ) -> bool
{
	try { 
		overlay::init( name );
	}
	catch (std::exception exception ) {
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

	status = D2D1CreateFactory( D2D1_FACTORY_TYPE_MULTI_THREADED, __uuidof( ID2D1Factory ), reinterpret_cast<void**>( &objects::factory ));
	if ( FAILED( status ) )
		return false;

	status = objects::d_device->QueryInterface( __uuidof( IDXGIDevice ), ( void** )( &objects::dxgi_device ) );
	if ( FAILED( status ) )
		return false;

	status = objects::factory->CreateDevice( objects::dxgi_device, &objects::device );
	if ( FAILED( status ) )
		return false;

	status = objects::device->CreateDeviceContext( D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &objects::ctx );
	if ( FAILED( status ) )
		return false;

	D2D1_PIXEL_FORMAT format{ DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE }; // Change if alpha is used in the future

	D2D1_SIZE_U size = D2D1::SizeU( overlay::window::size.x, overlay::window::size.y );
	D2D1_HWND_RENDER_TARGET_PROPERTIES h_properties = D2D1::HwndRenderTargetProperties( overlay::window::handle, size );
	D2D1_RENDER_TARGET_PROPERTIES properties{ D2D1_RENDER_TARGET_TYPE_DEFAULT, format, NULL, NULL, D2D1_RENDER_TARGET_USAGE_NONE, D2D1_FEATURE_LEVEL_DEFAULT};
	
	status = objects::swapchain->GetBuffer( 0, IID_PPV_ARGS( &objects::backbuffer ) );
	if ( FAILED( status ) )
		return false;

	// Get a D2D surface from the DXGI back buffer to use as the D2D render target.
	//status = objects::ctx->CreateBitmapFromDxgiSurface( objects::backbuffer, &properties, &objects::bitmap );
	status = objects::factory->CreateHwndRenderTarget( &properties, &h_properties, &objects::target );
	if ( FAILED( status ) )
		return false;

	objects::ctx->CreateSolidColorBrush( D2D1::ColorF( D2D1::ColorF::Black ), &objects::brush );
	return true;
}

void renderer::begin( )
{
    // objects::ctx->BeginDraw( );
	objects::target->BeginDraw( );
	objects::target->Clear( );

	
	RECT area{ };
	POINT point{ };
	GetWindowRect( overlay::window::handle, &area );
	MapWindowPoints( overlay::window::target, HWND_DESKTOP, &point, 1 );
	overlay::window::position = { point.x, point.y };

;	SetWindowPos( overlay::window::target, overlay::window::handle, NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE );
	if ( !IsWindowVisible( overlay::window::handle ) )
		ShowWindow( overlay::window::handle, SW_NORMAL );
}

void renderer::end( )
{
	objects::target->EndDraw( );
	objects::swapchain->Present( 1, 0 );
}