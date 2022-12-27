#pragma once
#include <concepts>
#include <fstream>
#include <vector>
#include <memory>

// Windows related includes
#include <Windows.h>

// DirectX includes
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

#include "../window/window.h"

using namespace DirectX;
constexpr int MAX_VERTICES = 1024 * 4 * 3;

/*
TODO:
	- Make viewport update dynamically depending on where the overlay is positioned (wayyyy later)
	- Figure out how to use index buffer for efficiency
	- Find a faster method to get shader bytecode
    - Add checks to render_list to differenitate between primitives that require sets of vertices or just a standalone vertex
	- Add projection matrix so we can use our vertex shader to take screen coordinates instead of gay, confined Direct3D coordinates
	- Dynamically make first viewport based on overlay
	- fuck.... learn how geometry shaders work just so i can get thicknes....
*/

template <class T>
concept Releasable = requires(T object)
{
	std::is_base_of<IUnknown, T>::value;
	std::is_pointer<T>::value;
};

class render_list;
class renderer
{
public:
	struct vertex
	{
		float x, y;
		float r, g, b, a;
	};
	
private:

	// Device related objects
	ID3D11Device* m_device{ nullptr };
	IDXGISwapChain* m_swapchain{ nullptr };
	ID3D11DeviceContext* m_ctx{ nullptr };

	ID3D11RenderTargetView* m_target{ nullptr }; // Render target specifices our target (whether it be a surface, or the back buffer) for drawing colors/textures to
	ID3D11InputLayout* m_layout{ nullptr }; // Specifices how our vertices are laid out in memory
	ID3D11Buffer* m_vbuffer{ nullptr }; // Stores vertex data before it is transformed
	ID3D11Buffer* m_ibuffer{ nullptr }; // Used to efficiently index into our vertex buffer

	
	// Needed for transforming positions (vertex shader) and color (pixel shader)
	ID3D11VertexShader* m_vshader{ nullptr };
	ID3D11PixelShader* m_pshader{ nullptr };

	XMMATRIX m_projection{ };
	ID3D11Buffer* m_projectionbuffer{ nullptr }; // Used with vertex shader to transform screen coordinates into clip coordinates (for Direct3D)

	// Required for telling Direct3D where we should render
	D3D11_VIEWPORT m_viewport{ NULL };

	// Helper class for batch rendering
	std::unique_ptr<render_list> m_renderlist;

public:
	renderer(window& overlay);
	~renderer();

	bool create_device(window& overlay);
	bool create_target();
	bool create_buffer();
	bool create_shaders();

	// Necessary for creating and destroying render frames
	auto update(window& overlay) -> void;
	auto begin() -> bool;
	auto end() -> void;

	// Getters & utility
	ID3D11Device* get_device();
	ID3D11DeviceContext* get_context();

	template <class T>
	static void safe_release(T object) requires Releasable<T>;

	// Drawing functions
	void draw_line(XMFLOAT2 from, XMFLOAT2 to, XMFLOAT3 color, float thickness);
	void draw_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness = 0.0f);
	void draw_filled_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness = 0.0f);
	void draw_point(XMFLOAT2 position, XMFLOAT3 color);
};


/*
credits to Yazzn on UC for the idea of batch rendering 
How does the render list work (for future me, when I am unable to read my own code)

Instead of rendering at different points in time, we render all the vertices at once in the renderer::end method
The batch and vertex vectors are synced, atleast in the theory. 
For every vertex or list of vertices, there should be a corresponding batch structure containing the data and amount of vertices.
By pairing these two together, we can dynamically tell Direct3D what we are rendering and how much of it.
*/

class render_list
{
public:
	render_list() = default;

	friend class renderer;
	struct batch
	{
		D3D_PRIMITIVE_TOPOLOGY topology;
		size_t count;
	};

	void add_vertices(D3D_PRIMITIVE_TOPOLOGY topology, std::vector<renderer::vertex> vertices);
	void add_vertex(D3D_PRIMITIVE_TOPOLOGY topology, renderer::vertex vertex);

	void clear()
	{
		m_batches.clear();
		m_vertices.clear();
	}

protected:
	std::vector<render_list::batch> m_batches{ };
	std::vector<renderer::vertex> m_vertices;
};

template <class T>
void renderer::safe_release(T object) requires Releasable<T>
{
	if (object)
		object->Release();
}
