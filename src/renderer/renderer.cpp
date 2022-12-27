#include "renderer.h"

renderer::renderer(window& overlay)
{
	if (!create_device(overlay))
		throw std::runtime_error("Failed to create DirectX device!");

	if (!create_target())
		throw std::runtime_error("Failed to create render target!");

	if (!create_shaders())
		throw std::runtime_error("Failed to initalize render shaders!");

	if (!create_buffer())
		throw std::runtime_error("Failed to create the vertex and/or index buffers!");

	m_renderlist = std::make_unique<render_list>();
}

renderer::~renderer()
{
	
	safe_release(m_pshader);
	safe_release(m_vshader);
	safe_release(m_ibuffer);
	safe_release(m_vbuffer);
	safe_release(m_layout);
	safe_release(m_target);
	safe_release(m_ctx);
	safe_release(m_swapchain);
	safe_release(m_device);
}

// Functions responsible for device, target, buffer and shader setup

// todo: dynamically make first viewport based on overlay
bool renderer::create_device(window& overlay)
{
	DXGI_SWAP_CHAIN_DESC chain_descriptor{ NULL };
	chain_descriptor.BufferCount = 1;
	chain_descriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chain_descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chain_descriptor.OutputWindow = overlay.get_hwnd();
	chain_descriptor.SampleDesc.Count = 1;
	chain_descriptor.Windowed = true;

	auto status = D3D11CreateDeviceAndSwapChain(
		NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		NULL,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&chain_descriptor,
		&m_swapchain,
		&m_device,
		NULL,
		&m_ctx
	);

	if (FAILED(status))
		return false;

	auto viewport = CD3D11_VIEWPORT(0.f, 0.f, 800.f, 600.0f);
	m_ctx->RSSetViewports(1, &viewport);


	return true;
}

bool renderer::create_target()
{
	// Create a new texture and set our render target to that texture
	ID3D11Texture2D* back_buffer{ nullptr };
	auto result = m_swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back_buffer));
	if (FAILED(result))
		return false;

	result = m_device->CreateRenderTargetView(back_buffer, nullptr, &m_target);
	if (FAILED(result))
		return false;

	safe_release(back_buffer);
	return true;
}

// TODO: Fix index buffer not working & dynamically create viewport from overlay
bool renderer::create_buffer()
{
	D3D11_BUFFER_DESC vertex_descriptor{ NULL };
	vertex_descriptor.ByteWidth = sizeof(renderer::vertex) * static_cast<UINT>(MAX_VERTICES);
	vertex_descriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertex_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vertex_descriptor.MiscFlags = 0;
	vertex_descriptor.Usage = D3D11_USAGE_DYNAMIC;

	auto result = m_device->CreateBuffer(&vertex_descriptor, NULL, &m_vbuffer);
	if (FAILED(result))
		return false;

	D3D11_BUFFER_DESC index_descriptor{ NULL };
	index_descriptor.ByteWidth = static_cast<UINT>(MAX_VERTICES);
	index_descriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

	result = m_device->CreateBuffer(&index_descriptor, NULL, &m_ibuffer);
	if (FAILED(result))
		return false;

	D3D11_VIEWPORT viewport{};
	UINT count = 1;
	m_ctx->RSGetViewports(&count, &viewport);

	m_projection = XMMatrixOrthographicOffCenterLH(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY,
		viewport.MinDepth, viewport.MaxDepth);

	D3D11_BUFFER_DESC matrix_descriptor{ NULL };
	matrix_descriptor.Usage = D3D11_USAGE_DYNAMIC;
	matrix_descriptor.ByteWidth = sizeof(XMMATRIX);
	matrix_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrix_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrix_descriptor.MiscFlags = 0;

	result = m_device->CreateBuffer(&matrix_descriptor, nullptr, &m_projectionbuffer);
	if (FAILED(result))
		return false;

	D3D11_MAPPED_SUBRESOURCE resource;
	m_ctx->Map(m_projectionbuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	std::memcpy(resource.pData, &m_projection, sizeof(XMMATRIX));
	m_ctx->Unmap(m_projectionbuffer, 0);
	
	return true;
}

// TODO: Find a faster method to get shader bytecode
bool renderer::create_shaders()
{
	std::ifstream vshader{ "vertex.cso", std::ios::binary };
	if (!vshader)
		return false;

	std::ifstream pshader{ "pixel.cso", std::ios::binary };
	if (!pshader)
		return false;

	std::vector<char> v_bytecode{ std::istreambuf_iterator<char>(vshader), std::istreambuf_iterator<char>() };
	std::vector<char> p_bytecode{ std::istreambuf_iterator<char>(pshader), std::istreambuf_iterator<char>() };

	auto result = m_device->CreateVertexShader(v_bytecode.data(), v_bytecode.size(), nullptr, &m_vshader);
	if (FAILED(result))
		return false;

	result = m_device->CreatePixelShader(p_bytecode.data(), p_bytecode.size(), nullptr, &m_pshader);
	if (FAILED(result))
		return false;

	// Create input layouts to tell Direct3D how our data is laid out in memory
	D3D11_INPUT_ELEMENT_DESC layout[]
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	m_device->CreateInputLayout(layout, 2, v_bytecode.data(), v_bytecode.size(), &m_layout);
	return true;
}

auto renderer::begin() -> bool
{
	const UINT stride = sizeof(renderer::vertex);
	const UINT offset = 0;

	m_ctx->OMSetRenderTargets(1, &m_target, nullptr);
	m_ctx->VSSetConstantBuffers(0, 1, &m_projectionbuffer);

	m_ctx->IASetInputLayout(m_layout);
	m_ctx->VSSetShader(m_vshader, nullptr, NULL);
	m_ctx->PSSetShader(m_pshader, nullptr, NULL);

	m_ctx->IASetVertexBuffers(0, 1, &m_vbuffer, &stride, &offset);
	m_ctx->IASetIndexBuffer(m_ibuffer, DXGI_FORMAT_R32_UINT, 0);

	return true;
}

// TODO: Fix index buffer not working
auto renderer::end() -> void
{
	// Copy our vertices from our render list into our vertex buffer (using ID3D11DeviceContext::Map)
	auto count = m_renderlist.get()->m_vertices.size();
	if (!count)
		return;

	D3D11_MAPPED_SUBRESOURCE resource{ };
	m_ctx->Map(m_vbuffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &resource);
	std::memcpy(resource.pData, m_renderlist.get()->m_vertices.data(), count * sizeof(renderer::vertex));
	m_ctx->Unmap(m_vbuffer, 0);

	size_t position = 0;
	for (const auto& batch : m_renderlist.get()->m_batches)
	{
		m_ctx->IASetPrimitiveTopology(batch.topology);
		m_ctx->Draw(batch.count, position);

		position += batch.count;
		//m_ctx->DrawIndexed(batch.count, 0, 0);
	}

	m_swapchain->Present(1, 0);
	m_renderlist.get()->clear();
}

// TODO: Make viewport update dynamically
auto renderer::update(window& overlay) -> void
{
	auto viewport = CD3D11_VIEWPORT(0.f, 0.f, 800.f, 600.0f);
	m_ctx->RSSetViewports(1, &viewport);

	const FLOAT color[]{ 0.25f, 0.5f, 1.0f, 1.0f };
	m_ctx->ClearRenderTargetView(m_target, color);
}

// Getters & drawing functions
ID3D11Device* renderer::get_device() 
{ 
	return m_device; 
}
ID3D11DeviceContext* renderer::get_context() 
{ 
	return m_ctx; 
}

void renderer::draw_line(XMFLOAT2 from, XMFLOAT2 to, XMFLOAT3 color, float thickness)
{
	// fix rotation/angle to make perfect thickness

	std::vector<renderer::vertex> vertices {
		{ from.x, from.y + thickness, color.x, color.y, color.z},
		{ from.x, from.y - thickness, color.x, color.y, color.z},
		{ to.x, to.y + thickness, color.x, color.y, color.z},
		{ to.x, to.y - thickness, color.x, color.y, color.z},
	};

	m_renderlist.get()->add_vertices(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP, vertices);
}

void renderer::draw_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness)
{
	draw_line({ position.x, position.y }, { position.x + width, position.y }, color, thickness);
	// draw_line({ position.x + width, position.y }, { position.x + width, position.y + height }, color, thickness);
	draw_line({ position.x + width, position.y + height }, { position.x, position.y + height }, color, thickness);
	// draw_line({ position.x, position.y + height }, { position.x, position.y }, color, thickness);
}

void renderer::draw_filled_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness)
{

}

void renderer::draw_point(XMFLOAT2 position, XMFLOAT3 color)
{

}



// renderlist functions

// TODO: Add checks to differenitate between primitives that require sets of vertices or just a standalone vertexd
void render_list::add_vertex(D3D_PRIMITIVE_TOPOLOGY topology, renderer::vertex vertex)
{
	auto batch = render_list::batch(topology, 1);
	m_vertices.emplace_back(vertex);
	m_batches.emplace_back(batch);
}

void render_list::add_vertices(D3D_PRIMITIVE_TOPOLOGY topology, std::vector<renderer::vertex> vertices)
{
	auto count = vertices.size();
	auto batch = render_list::batch(topology, count);

	for (const auto& vertex : vertices)
		m_vertices.emplace_back(vertex);

	m_batches.emplace_back(batch);
}
