cbuffer m_projectionbuffer : register(b0)
{
	matrix projection;
}

struct Input
{
	float3 position : SV_Position;
	float4 color : COLOR;
};

struct Output
{
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

Output main(Input input)
{
	Output output;
	output.position = mul(projection, float4(input.position.x, input.position.y, 0, 1));
	output.color = input.color;

	return output;
}