# ape-overlay
An external overlay that utilizes DirectX 11; made as a project to learn DirectX 11.


## TODO
  - Make the overlay *actually* an overlay
  - Fix wndproc handler on wm_quit and other shit
  - Make viewport update dynamically depending on where the overlay is positioned
  	- Dynamically make first viewport based on overlay 
  - Fix renderer not drawing multiple objects
  - Add checks to render_list to differenitate between primitives that require sets of vertices or just a standalone vertex
	- Figure out how to use index buffer for efficiency when rendering
  - Find a faster method to get shader bytecode	
  - Fix rotation for line thickness
  - Cleanup codebase
  
## Notes
```cpp
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
```
### How does the render list work (for future me, when I am unable to read my own code)

Instead of rendering at different points in time, we render all the vertices at once in the ``renderer::end`` method.
Tbatch and vertex vectors are synced, atleast in theory. For every vertex or list of vertices, there should be a corresponding batch structure containing the data and amount of vertices. By pairing these two together, we can dynamically tell Direct3D what we are rendering and how much of it.
