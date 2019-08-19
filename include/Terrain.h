#pragma once
#include <DDSTextureLoader.h>
#include "d3dx11effect.h"
#include "DirectXTex.h"
#include "DXUT.h"

#include "ConfigParser.h"
#include "SimpleImage.h"
#include "GameEffect.h"

struct SimpleVertex {
	DirectX::XMFLOAT2 UV;
	DirectX::XMFLOAT4 Pos;
	DirectX::XMFLOAT4 Normal;
};

struct Vector3d {
	float delx;
	float dely;
	float delz;
	float size;
	void normalize() {
		size = sqrt(delx * delx + dely * dely + 1);
		delx = delx / size;
		dely = dely / size;
		delz = 1 / size;
	}
	float slope() {
		return sqrt(delx * delx + dely * dely);
	}
};

class Terrain
{
public:
	Terrain(void);
	
	~Terrain(void);

	void destroy();
	
	HRESULT create(ID3D11Device* device, ConfigParser* configParser);
	
	void render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass);
	
private:
	Terrain(const Terrain&);

	Terrain(const Terrain&&);
	
	void fillIndexArray(std::vector<int>& index, ConfigParser* configParser);
	
	void fillHeightArray(std::vector<float>& heightField, ConfigParser* configParser);
	
	void fillVertexArray(std::vector<SimpleVertex>& vertex, ConfigParser* configParser);

	void CalculateNormal(std::vector<SimpleVertex>& vertex, ConfigParser* configParser);

	// Terrain rendering resources
	//Resource Constants
	UINT num_verticies    = 0;
	UINT num_indices      = 0;
	UINT field_resolution = 0;
	UINT width            = 0;

	ID3D11Buffer*                           indexBuffer;	   // Triangulation scheme of the terrain
	ID3D11Buffer*							heightBuffer;      // Height values of the terrain
	ID3D11ShaderResourceView*				heightBufferSRV;   // Describes the structure of the height buffer to the shader stages
	//ID3D11Texture2D*						normalTexture;	   // Describes the structure of the height buffer to the shader stages
	ID3D11ShaderResourceView*				normalTextureSRV;  // Describes the structure of the height buffer to the shader stages
	//ID3D11Texture2D*                      diffuseTexture;    // The terrain's material color for diffuse lighting
	ID3D11ShaderResourceView*               diffuseTextureSRV; // Describes the structure of the diffuse texture to the shader stages

};

