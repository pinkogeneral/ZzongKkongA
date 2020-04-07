#pragma once
#include "stdafx.h"
#include "Shader.h"


struct VS_CB_EYE_CAMERA_ORTHO
{
	XMFLOAT4X4						m_xmf4x4Ortho;
	XMFLOAT4X4						m_xmf4x4OrthoView;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPostProcessingShader : public CShader
{
public:

	CPostProcessingShader();
	virtual ~CPostProcessingShader();

	virtual D3D12_INPUT_LAYOUT_DESC CreateInputLayout();
	virtual D3D12_DEPTH_STENCIL_DESC CreateDepthStencilState();

	virtual D3D12_SHADER_BYTECODE CreateVertexShader(ID3DBlob** ppd3dShaderBlob);
	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

	virtual void CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);

	virtual void CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets = 1);

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		void* pContext = NULL, void* pLightContext = NULL, void* pShadowContext = NULL);

	virtual void ReleaseObjects();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	
	void GenerateOrthoLHMatrix(float fWidth, float fHeight, float fNearPlaneDistance, float fFarPlaneDistance);
	int GetUIControl() { return UIcontrol; }
	void SetUIControl(int iunder) { UIcontrol = iunder; }

protected:
	CTexture* m_pTexture = NULL;
	CTexture* m_pLightTexture = NULL;
	CTexture* m_pShadowTexture = NULL;

	CGameObject ** UIObject = NULL; 
	int m_nUI; 
	CUI* m_pCUIobj = NULL;
	int  UIcontrol = 1; 
	
	ID3D12Resource* m_pd3dcbvOrthoCamera = NULL;
	VS_CB_EYE_CAMERA_ORTHO* m_pcbMappedOrthoCamera = NULL;

	XMFLOAT4X4		             m_xmf4x4OrthoView;
	XMFLOAT4X4                   m_xmf4x4Ortho;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CPostProcessingByLaplacianShader : public CPostProcessingShader
{
public:
	CPostProcessingByLaplacianShader();
	virtual ~CPostProcessingByLaplacianShader();

	virtual D3D12_SHADER_BYTECODE CreatePixelShader(ID3DBlob** ppd3dShaderBlob);

};