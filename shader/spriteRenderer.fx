//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2DArray g_SpriteTex[5];
uint TextureArraySize = 5;
//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{

};

cbuffer cbChangesEveryFrame
{
	matrix  g_ViewProjection;
	float4  g_CameraUp;
	float4  g_CameraRight;
};

cbuffer cbUserChanges
{

};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState sampleAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState sampleLinearClamp
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Clamp;
	AddressV = Clamp;
};

//--------------------------------------------------------------------------------------
// Rasterizer states
//--------------------------------------------------------------------------------------

RasterizerState rsDefault {
};

RasterizerState rsCullFront {
	CullMode = Front;
};

RasterizerState rsCullBack {
	CullMode = Back;
};

RasterizerState rsCullNone {
	CullMode = None;
};

RasterizerState rsLineAA {
	CullMode = None;
	AntialiasedLineEnable = true;
};

//--------------------------------------------------------------------------------------
// DepthStates
//--------------------------------------------------------------------------------------

DepthStencilState EnableDepth
{
	DepthEnable = TRUE;
	DepthWriteMask = ALL;
	DepthFunc = LESS_EQUAL;
};

BlendState NoBlending
{
	AlphaToCoverageEnable = FALSE;
	BlendEnable[0] = FALSE;
};

BlendState BSBlendOver
{
	BlendEnable[0] = TRUE;
	SrcBlend[0] = SRC_ALPHA;
	SrcBlendAlpha[0] = ONE;
	DestBlend[0] = INV_SRC_ALPHA;
	DestBlendAlpha[0] = INV_SRC_ALPHA;
};

//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct SpriteVertex
{
	float3 Position     : POSITION;
	float  Radius       : RADIUS;
	uint   TextureIndex : INDEX;
	float  Animation    : ANIMATION;
	float  Opacity      : OPACITY;
};

struct PSVertex
{
	float4 Position	    : SV_POSITION;
	float2 TextureCoord : TEXCOORD;
	uint   TextureIndex : INDEX;
	float  Animation    : ANIMATION;
	float  Opacity      : OPACITY;
};

//--------------------------------------------------------------------------------------
// Vertex - Geometry - Pixel Shaders
//--------------------------------------------------------------------------------------
SpriteVertex SpriteVS(SpriteVertex vertex)
{
	return vertex;
}

[maxvertexcount(4)] 
void SpriteGS(point SpriteVertex vertex[1], inout TriangleStream<PSVertex> stream)
{
	PSVertex v = (PSVertex)0;

	float  r         = vertex[0].Radius;
	float3 pos       = vertex[0].Position;
	int    index     = vertex[0].TextureIndex;
	float  animation = vertex[0].Animation;
	float  opacity   = vertex[0].Opacity;
	
	v.Position     = mul(float4(pos - r * g_CameraRight.xyz + r * g_CameraUp.xyz, 1.0f), g_ViewProjection);
	v.TextureCoord = float2(0.0f, 0.0f);
	v.TextureIndex = index;
	v.Animation    = animation;
	v.Opacity      = opacity;
	stream.Append(v);

	v.Position     = mul(float4(pos + r * g_CameraRight.xyz + r * g_CameraUp.xyz, 1.0f), g_ViewProjection);
	v.TextureCoord = float2(1.0f, 0.0f);
	v.TextureIndex = index;
	v.Animation    = animation;
	v.Opacity      = opacity;
	stream.Append(v);

	v.Position     = mul(float4(pos - r * g_CameraRight.xyz - r * g_CameraUp.xyz, 1.0f), g_ViewProjection);
	v.TextureCoord = float2(0.0f, 1.0f);
	v.TextureIndex = index;
	v.Animation    = animation;
	v.Opacity      = opacity;
	stream.Append(v);

	v.Position     = mul(float4(pos + r * g_CameraRight.xyz - r * g_CameraUp.xyz, 1.0f), g_ViewProjection);
	v.TextureCoord = float2(1.0f, 1.0f);
	v.TextureIndex = index;
	v.Animation    = animation;
	v.Opacity      = opacity;
	stream.Append(v);


}

float4 SpritePS(PSVertex psv) : SV_Target0
{
	float3 size;
	float4 color;

	switch (psv.TextureIndex)
	{
	case 0:
		g_SpriteTex[0].GetDimensions(size.x, size.y, size.z);
		color = g_SpriteTex[0].Sample(sampleLinearClamp, float3(psv.TextureCoord, psv.Animation*size.z));

		return color * float4(1, 1, 1, psv.Opacity);
	case 1:
		g_SpriteTex[1].GetDimensions(size.x, size.y, size.z);
		color = g_SpriteTex[1].Sample(sampleLinearClamp, float3(psv.TextureCoord, psv.Animation*size.z));

		return color * float4(1, 1, 1, psv.Opacity);
	case 2:
		g_SpriteTex[2].GetDimensions(size.x, size.y, size.z);
		color = g_SpriteTex[2].Sample(sampleLinearClamp, float3(psv.TextureCoord, psv.Animation*size.z));

		return color * float4(1, 1, 1, psv.Opacity);
	case 3:
		g_SpriteTex[3].GetDimensions(size.x, size.y, size.z);
		color = g_SpriteTex[3].Sample(sampleLinearClamp, float3(psv.TextureCoord, psv.Animation*size.z));

		return color * float4(1, 1, 1, psv.Opacity);
	case 4:
		g_SpriteTex[4].GetDimensions(size.x, size.y, size.z);
		color = g_SpriteTex[4].Sample(sampleLinearClamp, float3(psv.TextureCoord, psv.Animation*size.z));

		return color * float4(1, 1, 1, psv.Opacity);
	default:
		return float4(0.5, 0.5, 0.5, 1);
	}
}


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, SpriteVS()));
		SetGeometryShader(CompileShader(gs_4_0, SpriteGS()));
		SetPixelShader(CompileShader(ps_4_0, SpritePS()));

		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(BSBlendOver, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
