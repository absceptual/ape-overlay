#include "renderer.h"

renderer::renderer(const wchar_t* process)
{
	try {
		m_overlay = std::make_unique<window>(process);
		m_process = process;
	}
	catch (std::runtime_error exception) { throw exception; }

	if (!create_device())
		throw std::runtime_error("Failed to create DirectX device!");

	if (!create_target())
		throw std::runtime_error("Failed to create render target!");

	if (!create_shaders())
		throw std::runtime_error("Failed to initalize render shaders!");

	if (!create_buffer())
		throw std::runtime_error("Failed to create the vertex and/or index buffers!");
}

// TODO: Add more releases	
renderer::~renderer()
{
	safe_release(m_device);
	safe_release(m_swapchain);
	safe_release(m_ctx);
	safe_release(m_pshader);
	safe_release(m_vshader);
	safe_release(m_ibuffer);
	safe_release(m_vbuffer);
	safe_release(m_layout);
	safe_release(m_target);
	safe_release(m_projectionbuffer);

	if (m_overlay)
		m_overlay.release();
}

bool renderer::create_device()
{
	DXGI_SWAP_CHAIN_DESC chain_descriptor{ NULL };
	chain_descriptor.BufferCount = 1;
	chain_descriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	chain_descriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	chain_descriptor.OutputWindow = m_overlay->get_hwnd();
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

	auto viewport = CD3D11_VIEWPORT(0.f, 0.f, window::width, window::height);
	m_ctx->RSSetViewports(1, &viewport);

	
	return true;
}

bool renderer::create_target()
{
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

bool renderer::create_buffer()
{
	D3D11_VIEWPORT viewport{};
	UINT count = 1;
	m_ctx->RSGetViewports(&count, &viewport);

	m_projection = XMMatrixOrthographicOffCenterLH(viewport.TopLeftX, viewport.Width, viewport.Height, viewport.TopLeftY, viewport.MinDepth, viewport.MaxDepth);

	D3D11_BUFFER_DESC matrix_descriptor{ NULL };
	matrix_descriptor.Usage = D3D11_USAGE_DYNAMIC;
	matrix_descriptor.ByteWidth = sizeof(XMMATRIX);
	matrix_descriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrix_descriptor.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrix_descriptor.MiscFlags = 0;

	auto result = m_device->CreateBuffer(&matrix_descriptor, nullptr, &m_projectionbuffer);
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
	m_effect = std::make_unique<BasicEffect>(m_device);
	m_effect->SetVertexColorEnabled(true);

	std::ifstream vshader{ "vertex.cso", std::ios::binary };
	std::ifstream pshader{ "pixel.cso", std::ios::binary };
	if (!vshader || !pshader)
		return false;

	std::vector<char> v_bytecode{ std::istreambuf_iterator<char>(vshader), std::istreambuf_iterator<char>() };
	std::vector<char> p_bytecode{ std::istreambuf_iterator<char>(pshader), std::istreambuf_iterator<char>() };

	auto result = m_device->CreateVertexShader(v_bytecode.data(), v_bytecode.size(), nullptr, &m_vshader);
	if (FAILED(result))
		return false;

	result = m_device->CreatePixelShader(p_bytecode.data(), p_bytecode.size(), nullptr, &m_pshader);
	if (FAILED(result))
		return false;

	CreateInputLayoutFromEffect<renderer::vertex_t>(m_device, m_effect.get(), &m_layout);

	m_pbatch = std::make_unique<PrimitiveBatch<vertex_t>>(m_ctx);
	m_effect->SetProjection(m_projection);
	m_effect->SetVertexColorEnabled(true);

	return true;
}

auto renderer::begin() -> bool
{
	m_ctx->OMSetRenderTargets(1, &m_target, nullptr);
	m_effect->Apply(m_ctx);
	m_ctx->VSSetConstantBuffers(0, 1, &m_projectionbuffer);

	m_ctx->IASetInputLayout(m_layout);
	m_ctx->VSSetShader(m_vshader, nullptr, NULL);
	m_ctx->PSSetShader(m_pshader, nullptr, NULL);

	float color[4]{ 0.f, 0.f, 0.f, 0.f };
	m_ctx->ClearRenderTargetView(m_target, color);

	m_pbatch->Begin();
	return true;
}

auto renderer::end(bool debug) -> void
{
	m_pbatch->End();
	m_swapchain->Present(0, 0);
}

// Updates drawing offset and rebuilds renderer if a change in screen resolution is detected
auto renderer::update(std::unique_ptr<renderer>& render) -> void
{
	auto overlay = render->get_overlay();

	RECT area{ };
	POINT point{ };
	GetWindowRect(overlay->get_hwnd(), &area);
	MapWindowPoints(overlay->get_target(), HWND_DESKTOP, &point, 1);

	window::offset = { point.x, point.y };

	SetWindowPos(overlay->get_target(), overlay->get_hwnd(), NULL, NULL, NULL, NULL, SWP_NOMOVE | SWP_NOSIZE);
	if (!IsWindowVisible(overlay->get_hwnd()))
		ShowWindow(overlay->get_hwnd(), SW_NORMAL);

	auto width = area.right - area.left;
	auto height = area.bottom - area.top;
	if (width != window::width || height != window::height)
	{
		/*
		auto process = render->m_process;
		try {
			render.release();
			render = std::make_unique<renderer>(process);
		}
		catch (std::runtime_error exception) {
			MessageBoxA(NULL, exception.what(), NULL, MB_ABORTRETRYIGNORE);
			exit(0);
		}
		*/
		;;
	}

}

// Getters & drawing functions


void renderer::draw_line(XMFLOAT2 from, XMFLOAT2 to, XMFLOAT3 color, float thickness)
{
	/// fix rotation / angle to make perfect thickness 
	from.x += window::offset.x;
	to.x += window::offset.x;

	from.y += window::offset.y;
	to.y += window::offset.y;

	vertex_t q1 = vertex_t(XMFLOAT3(from.x, from.y + thickness, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q2 = vertex_t(XMFLOAT3(from.x, from.y - thickness, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q3 = vertex_t(XMFLOAT3(to.x, to.y + thickness, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q4 = vertex_t(XMFLOAT3(to.x, to.y - thickness, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));

	m_pbatch->DrawQuad(q2, q4, q3, q1);
}

void renderer::draw_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness)
{
	position.x += window::offset.x;
	position.y += window::offset.y;

	vertex_t q1 = vertex_t(XMFLOAT3(position.x, position.y, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q2 = vertex_t(XMFLOAT3(position.x + width, position.y, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q3 = vertex_t(XMFLOAT3(position.x + width, position.y + height, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q4 = vertex_t(XMFLOAT3(position.x, position.y + height, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	m_pbatch->DrawLine(q1, q2);
	m_pbatch->DrawLine(q2, q3);
	m_pbatch->DrawLine(q3, q4);
	m_pbatch->DrawLine(q4, q1);
}

void renderer::draw_filled_box(XMFLOAT2 position, float width, float height, XMFLOAT3 color, float thickness)
{
	position.x += window::offset.x;
	position.y += window::offset.y;

	vertex_t q1 = vertex_t(XMFLOAT3(position.x, position.y, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q2 = vertex_t(XMFLOAT3(position.x + width, position.y, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q3 = vertex_t(XMFLOAT3(position.x + width, position.y + height, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	vertex_t q4 = vertex_t(XMFLOAT3(position.x, position.y + height, 0.f), XMFLOAT4(color.x, color.y, color.z, 1));
	m_pbatch->DrawQuad(q1, q2, q3, q4);
}

