#pragma once

// C++ related headers
#include <concepts>
#include <chrono>
#include <fstream>
#include <vector>
#include <memory>

// Windows related includes
#include <Windows.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <DirectXColors.h>
#include <DirectXHelpers.h>

#include <PrimitiveBatch.h>
#include <Effects.h>
#include <VertexTypes.h>

#include "../window/window.h"

using namespace DirectX;

template <class T>
concept Releasable = requires(T object)
{
	std::is_base_of<IUnknown, T>::value;
	std::is_pointer<T>::value;
};

class renderer
{
public:
	using vertex_t = VertexPositionColor;

	HINSTANCE m_instance{ };
	const wchar_t* m_process;

private:
	std::unique_ptr<window>					   m_overlay;
	std::unique_ptr<PrimitiveBatch<vertex_t>>  m_pbatch;
	std::unique_ptr<BasicEffect>			   m_effect;

	// Device related objects
	ID3D11Device* m_device					  { nullptr };
	IDXGISwapChain* m_swapchain				  { nullptr };
	ID3D11DeviceContext* m_ctx				  { nullptr };

	ID3D11RenderTargetView* m_target		  { nullptr }; 
	ID3D11InputLayout* m_layout				  { nullptr };
	ID3D11Buffer* m_vbuffer					  { nullptr }; 
	ID3D11Buffer* m_ibuffer					  { nullptr }; 

	ID3D11VertexShader* m_vshader			  { nullptr };
	ID3D11PixelShader* m_pshader			  { nullptr };

	XMMATRIX m_projection{ };
	ID3D11Buffer* m_projectionbuffer		  { nullptr };
	D3D11_VIEWPORT m_viewport				  { NULL };

public:
	renderer(const wchar_t* process);
	~renderer();

	bool create_device();
	bool create_target();
	bool create_buffer();
	bool create_shaders();

	static auto update(std::unique_ptr<renderer>& render) -> void;
	auto begin() -> bool;
	auto end(bool debug = false) -> void;

	ID3D11Device* get_device() { return m_device; }
	ID3D11DeviceContext* get_context() { return m_ctx; }
	window* get_overlay() { return m_overlay.get(); }

	template <class T>
	static void safe_release(T object) requires Releasable<T>;

	void draw_line(XMFLOAT2 from, XMFLOAT2 to, XMFLOAT3 color, float thickness = 1.0f);
	void draw_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness = 1.0f);
	void draw_filled_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness = 1.0f);
};

template <class T>
void renderer::safe_release(T object) requires Releasable<T>
{
	if (object)
		object->Release();
}
