#pragma once
#include <ctime>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <cassert>
#include <cstdint>
#include <algorithm>

#include <DXUT.h>
#include <DXUTcamera.h>
#include <DirectXTex.h>
#include <d3dx11effect.h>
#include <DDSTextureLoader.h>

#include "T3d.h"
#include "ConfigParser.h"
#include "D3DUtilities.h"


// Convenience macros for safe effect variable retrieval
#define SAFE_GET_PASS(Technique, name, var)              {assert(Technique!=NULL); var = Technique->GetPassByName( name );									 assert(var->IsValid());}
#define SAFE_GET_TECHNIQUE(effect, name, var)            {assert(effect!=NULL); var = effect->GetTechniqueByName( name );									 assert(var->IsValid());}
#define SAFE_GET_SCALAR(effect, name, var)               {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsScalar();						 assert(var->IsValid());}
#define SAFE_GET_VECTOR(effect, name, var)               {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsVector();						 assert(var->IsValid());}
#define SAFE_GET_MATRIX(effect, name, var)               {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsMatrix();						 assert(var->IsValid());}
#define SAFE_GET_SAMPLER(effect, name, var)              {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsSampler();						 assert(var->IsValid());}
#define SAFE_GET_RESOURCE(effect, name, var)             {assert(effect!=NULL); var = effect->GetVariableByName( name )->AsShaderResource();				 assert(var->IsValid());}
#define SAFE_GET_RESOURCE_ARRAY(effect, name, inex, var) {assert(effect!=NULL); var = effect->GetVariableByName( name )->GetElement(i)->AsShaderResource();  assert(var->IsValid());}


class Skybox
{
public:
	Skybox(const std::string& filename_t3d,  const std::string& filename_dds_cubemap,  float skyRadius);

	Skybox(const std::wstring& filename_t3d, const std::wstring& filename_dds_cubemap, float skyRadius);

	Skybox(const ConfigParser& configParser);
	
	~Skybox();

	HRESULT reloadShader(ID3D11Device* pDevice);
	
	void releaseShader();

	HRESULT create(ID3D11Device* pDevice);

	void destroy();

	void renderSkybox(ID3D11DeviceContext* context, const CFirstPersonCamera& camera);

private:
	ID3DX11Effect*			  m_pEffect;
	ID3DX11EffectTechnique*   m_pTechnique;
	ID3DX11EffectPass*        m_pEffectPass;
	ID3D11InputLayout*        m_pInputLayout;

	ID3D11Buffer*			  m_pIndexBuffer;
	ID3D11Buffer*             m_pVertexBuffer;
	
	ID3D11Texture2D*          m_pCubeTexture;
	ID3D11ShaderResourceView* m_pCubeSRV;

	std::wstring			  filename_dds_cubemap;
	std::wstring			  filename_t3d;
	float					  skyRadius;
	UINT					  m_pIndexCount;
	
};
