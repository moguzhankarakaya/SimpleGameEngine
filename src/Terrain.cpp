#include "Terrain.h"

// Macro to access array element
#define IDX(X,Y,WIDTH) ((X) + (Y) * (WIDTH))

Terrain::Terrain(void):
	indexBuffer(nullptr),
	heightBuffer(nullptr),
	heightBufferSRV(nullptr),
	diffuseTextureSRV(nullptr),
	normalTextureSRV(nullptr)
	//diffuseTexture(nullptr),
	//normalTexture(nullptr),
{
}

Terrain::~Terrain(void)
{
}

HRESULT Terrain::create(ID3D11Device* device, ConfigParser* configParser)
{
	HRESULT hr;

	// Note: For each vertex 10 floats are stored.

    // Note: In the coordinate system of the vertex buffer (output):
    // x = east,    y = up,    z = south,  
	// x, y, z  in  [0,1] (float)

	// Create buffer for heightBuffer
	std::vector<float> heightField;
	fillHeightArray(heightField, configParser);
	field_resolution = UINT(heightField.size());

	// Create index buffer
	std::vector<int> indices;
	fillIndexArray(indices, configParser);
	num_indices = UINT(indices.size());


    D3D11_SUBRESOURCE_DATA id;
    id.SysMemPitch      = sizeof(float); // Stride
    id.SysMemSlicePitch = 0;
    id.pSysMem          = &heightField[0];

    D3D11_BUFFER_DESC bd;
    bd.Usage          = D3D11_USAGE_DEFAULT;
    bd.BindFlags      = D3D11_BIND_SHADER_RESOURCE;
	bd.ByteWidth      = field_resolution * sizeof(float);    //The size in bytes of the triangle array
    bd.CPUAccessFlags = 0;
    bd.MiscFlags      = 0;

    V(device->CreateBuffer(&bd, &id, &heightBuffer)); // http://msdn.microsoft.com/en-us/library/ff476899%28v=vs.85%29.aspx

	// Create D3D11_SHADER_RESOURCE_VIEW_DESC 
	// to intiate heightBufferSRV member.
	D3D11_BUFFER_SRV bfr_srv;
	bfr_srv.FirstElement = 0;
	bfr_srv.NumElements  = field_resolution;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd;
	srvd.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvd.Format        = DXGI_FORMAT_R32_FLOAT;
	srvd.Buffer        = bfr_srv;
	
	V(device->CreateShaderResourceView(heightBuffer, &srvd, &heightBufferSRV));
	
	// Create and fill description
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage          = D3D11_USAGE_DEFAULT;
	bd.BindFlags      = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth      = sizeof(unsigned int) * num_indices;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags      = 0;

	// Define initial data
	ZeroMemory(&id, sizeof(id));
	id.pSysMem = &indices[0];
	
	// Create Buffer
	V(device->CreateBuffer(&bd, &id, &indexBuffer));

	// Load color texture (color map)
	// Load the color texture and create
	// the texture "diffuseTexture" as well as the shader resource view
	// "diffuseTextureSRV"

	std::wstring color_field = configParser->getTerrainPath(TerrainInfo::Texture::COLOR);
	V(DirectX::CreateDDSTextureFromFile(device, color_field.c_str(), nullptr, &diffuseTextureSRV));

	//Assignment 5 Loading normal texture
	std::wstring normal_field = configParser->getTerrainPath(TerrainInfo::Texture::NORMAL);
	V(DirectX::CreateDDSTextureFromFile(device, normal_field.c_str(), nullptr, &normalTextureSRV));

	if (hr != S_OK) {
		MessageBoxA(NULL, "Could not load texture \"resources\\terrain_color.dds\"", "Invalid texture", MB_ICONERROR | MB_OK);
		return hr;
	}

	return hr;
}


void Terrain::destroy()
{
    // Release the index buffer
    // Release the terrain's shader resource view and texture
	//SAFE_RELEASE(normalTexture);
	//SAFE_RELEASE(diffuseTexture);
	SAFE_RELEASE(indexBuffer);
	SAFE_RELEASE(heightBuffer);
	SAFE_RELEASE(diffuseTextureSRV);
	SAFE_RELEASE(heightBufferSRV);
	SAFE_RELEASE(normalTextureSRV);
}


void Terrain::render(ID3D11DeviceContext* context, ID3DX11EffectPass* pass)
{
	HRESULT hr;

	// Bind the terrain vertex buffer to the input assembler stage 
	ID3D11Buffer* vbs[]    = { nullptr, };
    unsigned int strides[] = { 0, }, offsets[] = { 0, };
    context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	
	// Bind the terrain index buffer to the input assembler stage
    // Tell the input assembler stage which primitive topology to use
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);    

    // Bind the SRV of the terrain diffuse texture to the effect variable
    V(g_gameEffect.diffuseEV->SetResource(diffuseTextureSRV));
	V(g_gameEffect.heightEV->SetResource(heightBufferSRV));
	V(g_gameEffect.normalEV->SetResource(normalTextureSRV));
	V(g_gameEffect.resolutionEV->SetInt(width));
	
    // Apply the rendering pass in order to submit the necessary render state changes to the device
    V(pass->Apply(0, context));

    // Draw
    // Call DrawIndexed to draw the terrain geometry using as shared vertex list
    context->DrawIndexed(num_indices, 0, 0);
}

void Terrain::fillHeightArray(std::vector<float>& heightField, ConfigParser* configParser) 
{	
	std::wstring height_field = configParser->getTerrainPath(TerrainInfo::Texture::HEIGHTMAP);
	GEDUtils::SimpleImage image(height_field.c_str());
	UINT RES = image.getWidth();
	width = RES;
	heightField.resize(RES*RES);

	for (UINT i = 0; i < RES; i++) {
		for (UINT j = 0; j < RES; j++) {

			heightField[IDX(i, j, RES)] = image.getPixel(i, j);

		}
	}
}

void Terrain::fillVertexArray(std::vector<SimpleVertex>& vertex, ConfigParser* configParser) 
{
	std::wstring height_field = configParser->getTerrainPath(TerrainInfo::Texture::HEIGHTMAP);
	GEDUtils::SimpleImage image(height_field.c_str());
	UINT RES = image.getWidth();
	vertex.resize(RES*RES);
	float width = configParser->getTerrainWidth();
	float depth = configParser->getTerrainDepth();
	float height = configParser->getTerrainHeight();
	
	for (UINT i = 0; i < RES; i++) {
		for (UINT j = 0; j < RES; j++) {
			vertex[IDX(i, j, RES)].Pos.x = (float(i) / RES - float(0.5))*width;
			vertex[IDX(i, j, RES)].Pos.y = image.getPixel(i, j) * height;
			vertex[IDX(i, j, RES)].Pos.z = (float(j) / RES - float(0.5))*depth;
			vertex[IDX(i, j, RES)].Pos.w = 1;
			vertex[IDX(i, j, RES)].UV.x = float(i) / (RES - 1);
			vertex[IDX(i, j, RES)].UV.y = float(j) / (RES - 1);
		}
	}
	CalculateNormal(vertex, configParser);
}

void Terrain::fillIndexArray(std::vector<int>& index, ConfigParser* configParser) {

	std::wstring height_field = configParser->getTerrainPath(TerrainInfo::Texture::HEIGHTMAP);
	GEDUtils::SimpleImage image(height_field.c_str());
	UINT RES = image.getWidth();

	for (UINT i = 0; i < RES - 1; i++) {
		for (UINT j = 0; j < RES - 1; j++) {
			index.push_back(IDX(i, j, RES));
			index.push_back(IDX(i, j + 1, RES));
			index.push_back(IDX(i + 1, j, RES));
			index.push_back(IDX(i + 1, j, RES));
			index.push_back(IDX(i, j + 1, RES));
			index.push_back(IDX(i + 1, j + 1, RES));
		}
	}

}

void Terrain::CalculateNormal(std::vector<SimpleVertex>& vertex, ConfigParser* configParser) {
	
	std::wstring height_field = configParser->getTerrainPath(TerrainInfo::Texture::HEIGHTMAP);
	GEDUtils::SimpleImage image(height_field.c_str());
	UINT res = image.getWidth();
	
	int _side = res;
	int _size = res * res;

	std::vector<float> terrain(_size);
	for (int i = 0; i < _side; i++) {
		for (int j = 0; j < _side; j++) {
			terrain[IDX(i, j, _side)] = image.getPixel(i, j);
		}
	}

	Vector3d normalVector;

	float stepSize = float(1.0) / (_side);

	for (UINT x = 0; x < res; x++) {
		for (UINT y = 0; y < res; y++) {
			if (x == 0)
				normalVector.delx = -1 * (terrain[IDX(x + 1, y, _side)] - terrain[IDX(x, y, _side)]) / (stepSize);
			else if (x == res - 1)
				normalVector.delx = -1 * (terrain[IDX(x, y, _side)] - terrain[IDX(x - 1, y, _side)]) / (stepSize);
			else
				normalVector.delx = -1 * (terrain[IDX(x + 1, y, _side)] - terrain[IDX(x - 1, y, _side)]) / (2 * stepSize);
			if (y == 0)
				normalVector.dely = -1 * (terrain[IDX(x, y + 1, _side)] - terrain[IDX(x, y, _side)]) / (stepSize);
			else if (y == res - 1)
				normalVector.dely = -1 * (terrain[IDX(x, y, _side)] - terrain[IDX(x, y - 1, _side)]) / (stepSize);
			else
				normalVector.dely = -1 * (terrain[IDX(x, y + 1, _side)] - terrain[IDX(x, y - 1, _side)]) / (2 * stepSize);

			normalVector.delz = 1;
			normalVector.normalize();
			vertex[IDX(x, y, _side)].Normal.x = normalVector.delx;
			vertex[IDX(x, y, _side)].Normal.y = normalVector.dely;
			vertex[IDX(x, y, _side)].Normal.z = normalVector.delz;
		}
	}
}