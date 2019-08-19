#include "Skybox.h"

Skybox::Skybox(const std::string& _filename_t3d, const std::string& _filename_dds_cubemap, float skyRadius)
	:	skyRadius(skyRadius)
{
	filename_t3d		 = std::wstring(_filename_t3d.begin(),		   _filename_t3d.end());
	filename_dds_cubemap = std::wstring(_filename_dds_cubemap.begin(), _filename_dds_cubemap.end());
	
	m_pEffect	   = nullptr;  m_pTechnique	   = nullptr;
	m_pEffectPass  = nullptr;  m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;  m_pCubeTexture  = nullptr;
	m_pCubeSRV     = nullptr;
	
	m_pIndexCount = 0;
}

Skybox::Skybox(const std::wstring& filename_t3d, const std::wstring& filename_dds_cubemap, float skyRadius)
	:	filename_t3d(filename_t3d), filename_dds_cubemap(filename_dds_cubemap), skyRadius(skyRadius)
{
	m_pEffect		= nullptr;  m_pTechnique    = nullptr; 
	m_pEffectPass	= nullptr;	m_pVertexBuffer = nullptr; 
	m_pIndexBuffer  = nullptr;  m_pCubeTexture  = nullptr; 
	m_pCubeSRV	    = nullptr;
	
	m_pIndexCount = 0;
}

Skybox::Skybox(const ConfigParser& configParser)
{
	SkyboxInfo skyboxInfo = configParser.getSkyboxInfo();

	m_pEffect	   = nullptr;  m_pTechnique    = nullptr;
	m_pEffectPass  = nullptr;  m_pVertexBuffer = nullptr;
	m_pIndexBuffer = nullptr;  m_pCubeTexture  = nullptr;
	m_pCubeSRV	   = nullptr;

	m_pIndexCount        = 0;
	skyRadius            = skyboxInfo.skyboxRadius;
	filename_t3d         = std::wstring(skyboxInfo.filename_t3d.begin(), skyboxInfo.filename_t3d.end());
	filename_dds_cubemap = std::wstring(skyboxInfo.filename_dds.begin(), skyboxInfo.filename_dds.end());
}

Skybox::~Skybox()
{

}

HRESULT Skybox::reloadShader(ID3D11Device* pDevice)
{
	Utils::loadEffect(pDevice, &m_pEffect, L"shader\\skybox.fxo");
	assert(m_pEffect->IsValid());

	return S_OK;
}

void Skybox::releaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT Skybox::create(ID3D11Device* pDevice)
{
	HRESULT hr;

	D3D11_BUFFER_DESC bd = { 0 };
	D3D11_SUBRESOURCE_DATA id = { 0 };

	std::vector<DirectX::XMFLOAT3> vertexBufferData;
	std::vector<T3dVertex> T3dVertexBuffer;
	std::vector<UINT>  indexBufferData;
	V(T3d::readFromFile(filename_t3d, T3dVertexBuffer, indexBufferData));
	for (auto& T3dV : T3dVertexBuffer)
	{
		DirectX::XMFLOAT3 vertexPos;
		DirectX::XMVECTOR scaledDome = DirectX::XMVectorScale(DirectX::XMVectorSet(T3dV.position.x, T3dV.position.y, T3dV.position.z, 1.0f), skyRadius);
		DirectX::XMStoreFloat3(&(vertexPos), scaledDome);
		vertexBufferData.push_back(vertexPos);
	}

	m_pIndexCount = UINT(indexBufferData.size());

	id.pSysMem			   = &vertexBufferData[0];
	bd.Usage			   = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags		   = D3D11_BIND_VERTEX_BUFFER;
	bd.ByteWidth		   = UINT(vertexBufferData.size() * sizeof(DirectX::XMFLOAT3));
	bd.CPUAccessFlags	   = 0;
	bd.MiscFlags		   = 0;
	bd.StructureByteStride = 0;

	V(pDevice->CreateBuffer(&bd, &id, &m_pVertexBuffer));

	ZeroMemory(&id, sizeof(id));
	ZeroMemory(&bd, sizeof(bd));

	id.pSysMem			   = &indexBufferData[0];
	bd.Usage			   = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags		   = D3D11_BIND_INDEX_BUFFER;
	bd.ByteWidth		   = sizeof(unsigned int) * m_pIndexCount;
	bd.CPUAccessFlags	   = 0;
	bd.MiscFlags	       = 0;
	bd.StructureByteStride = 0;


	V(pDevice->CreateBuffer(&bd, &id, &m_pIndexBuffer));

	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// Obtain the effect technique and effect pass
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", m_pTechnique);
	SAFE_GET_PASS(m_pTechnique, "P0", m_pEffectPass);

	// Create the input layout
	D3DX11_PASS_DESC pd;
	V_RETURN(m_pEffectPass->GetDesc(&pd));
	V_RETURN(pDevice->CreateInputLayout(layout, numElements, pd.pIAInputSignature, pd.IAInputSignatureSize, &m_pInputLayout));

	if (filename_dds_cubemap != TEXT("-"))
		V(Utils::createTexture(pDevice, filename_dds_cubemap, &m_pCubeTexture, &m_pCubeSRV));


	return S_OK;
}

void Skybox::destroy()
{
	SAFE_RELEASE(m_pCubeSRV);
	SAFE_RELEASE(m_pCubeTexture);
	SAFE_RELEASE(m_pIndexBuffer);
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	SAFE_RELEASE(m_pEffectPass);
	SAFE_RELEASE(m_pTechnique);
}

void Skybox::renderSkybox(ID3D11DeviceContext* context, const CFirstPersonCamera& camera)
{
	// center Sky about eye in world space
	DirectX::XMFLOAT3 eyePos;
	DirectX::XMStoreFloat3(&(eyePos), camera.GetEyePt());
	DirectX::XMMATRIX translation   = DirectX::XMMatrixTranslation(eyePos.x, eyePos.y, eyePos.z);
	DirectX::XMMATRIX worldViewProj = translation * camera.GetViewMatrix() * camera.GetProjMatrix();


	ID3DX11EffectMatrixVariable* ViewProjectionEV = nullptr;
	SAFE_GET_MATRIX(m_pEffect, "gWorldViewProj", ViewProjectionEV);
	ViewProjectionEV->SetMatrix((float*)&worldViewProj);

	ID3DX11EffectShaderResourceVariable* cubeMapSRV;
	SAFE_GET_RESOURCE(m_pEffect, "gCubeMap", cubeMapSRV);
	cubeMapSRV->SetResource(m_pCubeSRV);


	ID3D11Buffer* vbs[] = { m_pVertexBuffer, };
	unsigned int strides[] = { sizeof(DirectX::XMFLOAT3), }, offsets[] = { 0, };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
	context->IASetInputLayout(m_pInputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	D3DX11_TECHNIQUE_DESC techniqueDesc;
	m_pTechnique->GetDesc(&techniqueDesc);
	
	for (UINT p = 0; p < techniqueDesc.Passes; ++p)
	{
		ID3DX11EffectPass* pass = m_pTechnique->GetPassByIndex(p);

		pass->Apply(0, context);

		context->DrawIndexed(m_pIndexCount, 0, 0);
	}
}
