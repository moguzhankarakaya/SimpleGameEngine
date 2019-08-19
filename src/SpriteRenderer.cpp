#include "SpriteRenderer.h"

SpriteRenderer::SpriteRenderer(const std::vector<std::wstring>& textureFilenames) : 
	m_textureFilenames(textureFilenames)
{
	m_pEffect = nullptr;
	m_pTechnique = nullptr;
	m_pEffectPass = nullptr;
	m_pVertexBuffer = nullptr;
	m_pInputLayout = nullptr;

	m_spriteCountMax = 4096;
}

SpriteRenderer::~SpriteRenderer()
{
}

HRESULT SpriteRenderer::reloadShader(ID3D11Device * pDevice)
{
	Utils::loadEffect(pDevice, &m_pEffect, L"shader\\spriteRenderer.fxo");
	assert(m_pEffect->IsValid());
	return S_OK;
}

void SpriteRenderer::releaseShader()
{
	SAFE_RELEASE(m_pEffect);
}

HRESULT SpriteRenderer::create(ID3D11Device * pDevice)
{
	HRESULT hr;

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth      = UINT(m_spriteCountMax * sizeof(SpriteVertex));
	bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
	bd.Usage          = D3D11_USAGE_DEFAULT;
	bd.CPUAccessFlags = 0;
	bd.MiscFlags      = 0;

	V(pDevice->CreateBuffer(&bd, nullptr, &m_pVertexBuffer)); // http://msdn.microsoft.com/en-us/library/ff476899%28v=vs.85%29.aspx

														  // Define the input layout
	const D3D11_INPUT_ELEMENT_DESC layout[] = // http://msdn.microsoft.com/en-us/library/bb205117%28v=vs.85%29.aspx
	{
		{ "POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "RADIUS",      0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "OPACITY",     0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "ANIMATION",   0, DXGI_FORMAT_R32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "INDEX",       0, DXGI_FORMAT_R32_UINT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};
	UINT numElements = sizeof(layout) / sizeof(layout[0]);

	// Obtain the effect technique and effect pass
	SAFE_GET_TECHNIQUE(m_pEffect, "Render", m_pTechnique);
	SAFE_GET_PASS(m_pTechnique, "P0", m_pEffectPass);

	// Create the input layout
	D3DX11_PASS_DESC pd;
	V_RETURN(m_pEffectPass->GetDesc(&pd));
	V_RETURN(pDevice->CreateInputLayout(layout, numElements, pd.pIAInputSignature, pd.IAInputSignatureSize, &m_pInputLayout));
	
	int _effectTextureSize;
	ID3DX11EffectScalarVariable* EffectTextureSize;
	SAFE_GET_SCALAR(m_pEffect, "TextureArraySize", EffectTextureSize);
	EffectTextureSize->GetInt(&_effectTextureSize);
	
	for (int i = 0; i < m_textureFilenames.size(); i++)
	{
		if (i < _effectTextureSize)
		{
			const auto&        textureFile    = m_textureFilenames[i];
			ID3D11Texture2D * _texture        = nullptr;
			ID3D11ShaderResourceView *           _shaderResourceView  = nullptr;
			ID3DX11EffectShaderResourceVariable* _srvVariable         = nullptr;

			V_RETURN(Utils::createTexture(pDevice, textureFile, &_texture, &_shaderResourceView));
			SAFE_GET_RESOURCE_ARRAY(m_pEffect, "g_SpriteTex", i, _srvVariable);
			_srvVariable->SetResource(_shaderResourceView);

			m_pSpriteTex.push_back(_texture);
			m_pSpriteSRV.push_back(_shaderResourceView);
		}
	}

	return S_OK;
}

void SpriteRenderer::destroy()
{
	SAFE_RELEASE(m_pVertexBuffer);
	SAFE_RELEASE(m_pInputLayout);
	for (auto& m_texture : m_pSpriteTex)
		SAFE_RELEASE(m_texture);
	for (auto& m_srv : m_pSpriteSRV)
		SAFE_RELEASE(m_srv);
	SAFE_RELEASE(m_pEffectPass);
	SAFE_RELEASE(m_pTechnique);
}

HRESULT SpriteRenderer::renderSprites(ID3D11DeviceContext * context, const std::vector<SpriteVertex>& sprites, const CFirstPersonCamera & camera)
{
	HRESULT hr;

	UINT vertexCount = UINT(sprites.size());

	if (vertexCount == 0)
		return S_OK;
	
	D3D11_BOX destRegion;
	destRegion.left  = 0;      destRegion.right  = vertexCount * sizeof(SpriteVertex);
	destRegion.top   = 0;      destRegion.bottom = 1;
	destRegion.front = 0;      destRegion.back   = 1;
	
	const void* sourceData = nullptr;
	if (!sprites.empty())
		sourceData = &sprites[0];
	
	context->UpdateSubresource(m_pVertexBuffer, 0, &destRegion, sourceData, 0, 0);

	ID3D11Buffer* vbs[] = { m_pVertexBuffer, };
	unsigned int strides[] = { sizeof(SpriteVertex), }, offsets[] = { 0, };
	context->IASetVertexBuffers(0, 1, vbs, strides, offsets);
	
	context->IASetInputLayout(m_pInputLayout);
	context->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);

	// Affine transformation variables
	ID3DX11EffectVectorVariable* CameraUpEV       = nullptr;
	ID3DX11EffectVectorVariable* CameraRightEV    = nullptr;
	ID3DX11EffectMatrixVariable* ViewProjectionEV = nullptr;
	SAFE_GET_VECTOR(m_pEffect, "g_CameraUp", CameraUpEV);
	SAFE_GET_VECTOR(m_pEffect, "g_CameraRight", CameraRightEV);
	SAFE_GET_MATRIX(m_pEffect, "g_ViewProjection", ViewProjectionEV);

	DirectX::XMVECTOR const cameraUp    = camera.GetWorldUp();
	DirectX::XMVECTOR const cameraRight = camera.GetWorldRight();
	DirectX::XMMATRIX const g_ViewProj  = camera.GetViewMatrix() * camera.GetProjMatrix();

	ViewProjectionEV->SetMatrix((float*)&g_ViewProj);
	CameraUpEV->SetFloatVector((float*)&cameraUp);
	CameraRightEV->SetFloatVector((float*)&cameraRight);
	
	V(m_pEffectPass->Apply(0, context));
	context->Draw(vertexCount, UINT(0));

	return S_OK;
}
