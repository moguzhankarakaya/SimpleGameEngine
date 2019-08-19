//--------------------------------------------------------------------------------------
// Shader resources
//--------------------------------------------------------------------------------------

Texture2D     g_Diffuse;   // Material albedo for diffuse lighting
Texture2D     g_NormalMap; // Material normal map
Buffer<float> g_HeightMap; // Height map for terrain rendering
Texture2D     g_Specular;  // Material specular map
Texture2D	  g_Glow;	   // Material glow map

//--------------------------------------------------------------------------------------
// Constant buffers
//--------------------------------------------------------------------------------------

cbuffer cbConstant
{
	float4  g_LightDir;    // Object space
	int     g_TerrainRes;
};

cbuffer cbChangesEveryFrame
{
	matrix  g_World;
	matrix  g_WorldViewProjection;
	float   g_Time;
	matrix  g_WorldNormals;
	matrix  g_Mash;
	matrix  g_MeshViewProjection;
	matrix  g_MeshNormals;
	float4  g_CameraPassWorld;
};

cbuffer cbUserChanges
{
};


//--------------------------------------------------------------------------------------
// Structs
//--------------------------------------------------------------------------------------

struct PosNorTex
{
	float4 Pos : SV_POSITION;
	float4 Nor : NORMAL;
	float2 Tex : TEXCOORD;
};

struct PosTexLi
{
	float4   Pos : SV_POSITION;
	float2	 Tex : TEXCOORD;
	float	  Li : LIGHT_INTENSITY;
	float3 normal: NORMAL;
};

struct PosTex
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD;
};

struct T3dVertexVSIn 
{
	float3 Pos : POSITION;  //Position in object space 
	float2 Tex : TEXCOORD;  //Texture coordinate 
	float3 Nor : NORMAL;	//Normal in object space 
	float3 Tan : TANGENT;	//Tangent in object space (not used in Ass. 5) 
}; 

struct T3dVertexPSIn 
{ 
	float4 Pos : SV_POSITION;	//Position in clip space 
	float2 Tex : TEXCOORD;		//Texture coordinate 
	float3 PosWorld : POSITION; //Position in world space 
	float3 NorWorld : NORMAL;	//Normal in world space 
	float3 TanWorld : TANGENT;	//Tangent in world space (not used in Ass. 5) 
};

//--------------------------------------------------------------------------------------
// Samplers
//--------------------------------------------------------------------------------------

SamplerState samAnisotropic
{
	Filter = ANISOTROPIC;
	AddressU = Wrap;
	AddressV = Wrap;
};

SamplerState samLinearClamp
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


//--------------------------------------------------------------------------------------
// Shaders
//--------------------------------------------------------------------------------------

// Simple VS for testing
PosTexLi SimpleVS(PosNorTex Input) {
	PosTexLi output = (PosTexLi)0;

	// Transform position from object space to homogenious clip space
	output.Pos = mul(Input.Pos, g_WorldViewProjection);

	// Pass trough normal and texture coordinates
	output.Tex = Input.Tex;

	// Calculate light intensity
	output.normal = normalize(mul(Input.Nor, g_World).xyz); // Assume orthogonal world matrix
	output.Li = saturate(dot(output.normal, g_LightDir.xyz));

	return output;
}

// Simple PS for testing
float4 SimplePS(PosTexLi Input) : SV_Target0{
	// Perform lighting in object space, so that we can use the input normal "as it is"
	float4 matDiffuse = g_Diffuse.Sample(samLinearClamp, Input.Tex);
	return float4(matDiffuse.rgb * Input.Li, 1);
}


PosTex TerrainVS(in uint VertexID : SV_VertexID) {
	PosTex output = (PosTex)0;

	uint y = VertexID / g_TerrainRes;
	uint x = VertexID % g_TerrainRes;

	output.Pos.x = float(x) / g_TerrainRes - 0.5;
	output.Pos.y = g_HeightMap[VertexID];
	output.Pos.z = float(y) / g_TerrainRes - 0.5;
	output.Pos.w = 1;

	//Can we do it like that in one line???
	//output.Pos = float4(float(x) / g_TerrainRes - 0.5, g_HeightMap[VertexID], 
	//				      float(y) / g_TerrainRes - 0.5, 1);

	//g_WorldViewProjection Matrix includes scaling and trasnlastion
	output.Pos = mul(output.Pos, g_WorldViewProjection);

	output.Tex.x = float(x) / g_TerrainRes;
	output.Tex.y = float(y) / g_TerrainRes;

	//Can we do it like that in one line???
	//output.Tex = float2(x, y) / g_TerrainRes;

	return output;
}

float4 TerrainPS(PosTex Input) : SV_Target0 {
	float4 n;
	n.xz = (g_NormalMap.Sample(samLinearClamp, Input.Tex) - 0.5).xy * 2;
	n.y = sqrt(1 - (n.x*n.x + n.z*n.z));
	n.w = 0;

	n.xyz = normalize(mul(n, g_WorldNormals).xyz);
	
	float i = saturate(dot(n.xyz, g_LightDir.xyz)) * 0.95 + 0.05;

	float3 matDiffuse = g_Diffuse.Sample(samAnisotropic, Input.Tex).rgb;

	return float4(matDiffuse.rgb * i, 1);
}


T3dVertexPSIn MeshVS(T3dVertexVSIn Input) { 
	T3dVertexPSIn output;

	output.Pos	     = mul(float4(Input.Pos, 1), g_MeshViewProjection);
	output.Tex		 = Input.Tex;
	output.PosWorld  = mul(float4(Input.Pos, 1), g_Mash).xyz;
	output.NorWorld  = normalize(mul(float4(Input.Nor, 0), g_MeshNormals).xyz);
	output.TanWorld  = normalize(mul(float4(Input.Tan, 0), g_Mash).xyz);

	return output;
}

float4 MeshPS(T3dVertexPSIn Input) : SV_Target0 {
	float cd = 0.8;
	float cs = 0.5;
	float ca = 0.2;
	float cg = 0.5;
	float s = 9;
	float3 diffuse  = g_Diffuse.Sample(samAnisotropic, Input.Tex).rgb;
	float3 specular = g_Specular.Sample(samAnisotropic, Input.Tex).rgb;
	float3 glow     = g_Glow.Sample(samAnisotropic, Input.Tex).rbg;
	float3 light = float3(1, 1, 1);
	float3 ambientlight = float3(1, 1, 1);
	float3 n = normalize(Input.NorWorld);
	float3 I = (g_LightDir).xyz;
	float3 r = reflect(-1 * I, n);
	float3 v = normalize( ( ( g_CameraPassWorld).xyz - (Input.PosWorld) ).xyz );
	
	float3 result = cd * diffuse * saturate(dot(n, I)) * light + \
					cs * specular * pow(saturate(dot(r, v)), s) * light + \
					ca * diffuse * ambientlight + \
					cg * glow;
	return float4(result, 1);
}

//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------
technique11 Render
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, TerrainVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, TerrainPS()));

		SetRasterizerState(rsCullNone);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}

	pass P1_Mesh
	{
		SetVertexShader(CompileShader(vs_4_0, MeshVS()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, MeshPS()));

		SetRasterizerState(rsCullBack);
		SetDepthStencilState(EnableDepth, 0);
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
	}
}
