#include"stdafx.h"
#include"Geometry.h"
#include"Shader.h"



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CHeightMapTerrain::CHeightMapTerrain(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, LPCTSTR pFileName, int nWidth, int nLength, int nBlockWidth, int nBlockLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color)
{
	m_xmf4x4World = Matrix4x4::Identity();

	m_nWidth = nWidth;
	m_nLength = nLength;

	m_pxmf2TessFactor = new XMFLOAT2();
	int cxQuadsPerBlock = nBlockWidth - 1;
	int czQuadsPerBlock = nBlockLength - 1;

	m_xmf3Scale = xmf3Scale;

	m_pHeightMapImage = new CHeightMapImage(pFileName, nWidth, nLength, xmf3Scale);

	long cxBlocks = (m_nWidth - 1) / cxQuadsPerBlock;
	long czBlocks = (m_nLength - 1) / czQuadsPerBlock;

	m_nMeshes = cxBlocks * czBlocks;
	m_ppMeshes = new CMesh * [m_nMeshes];
	for (int i = 0; i < m_nMeshes; i++)	m_ppMeshes[i] = NULL;

	CHeightMapGridMesh* pHeightMapGridMesh = NULL;
	for (int z = 0, zStart = 0; z < czBlocks; z++)
	{
		for (int x = 0, xStart = 0; x < cxBlocks; x++)
		{
			xStart = x * (nBlockWidth - 1);
			zStart = z * (nBlockLength - 1);
			pHeightMapGridMesh = new CHeightMapGridMesh(pd3dDevice, pd3dCommandList, xStart, zStart, nBlockWidth, nBlockLength, xmf3Scale, xmf4Color, m_pHeightMapImage);
			SetMesh(x + (z * cxBlocks), pHeightMapGridMesh);
		}
	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
	*m_pxmf2TessFactor = XMFLOAT2(2, 2);
	CTexture* pTerrainTexture = new CTexture(2, RESOURCE_TEXTURE2D, 0);

	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/Base_Texture.dds", 0);
	pTerrainTexture->LoadTextureFromFile(pd3dDevice, pd3dCommandList, L"Image/DirtDetail.dds", 1);

	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	CTerrainShader* pTerrainShader = new CTerrainShader();
	pTerrainShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	pTerrainShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
	pTerrainShader->CreateCbvAndSrvDescriptorHeaps(pd3dDevice, pd3dCommandList, 1, 2);
	pTerrainShader->CreateShaderResourceViews(pd3dDevice, pd3dCommandList, pTerrainTexture, 4, true);
	pTerrainShader->CreateConstantBufferViews(pd3dDevice, pd3dCommandList, 1, m_pd3dcbGameObjects, ncbElementBytes);

	SetShader(pTerrainShader);

	m_ppMaterials[0]->SetTexture(pTerrainTexture);

	SetCbvGPUDescriptorHandle(pTerrainShader->GetGPUCbvDescriptorStartHandle());

}

CHeightMapTerrain::~CHeightMapTerrain(void)
{
	if (m_pHeightMapImage) delete m_pHeightMapImage;
}

void CHeightMapTerrain::SetMesh(int nIndex, CMesh* pMesh)
{
	if (m_ppMeshes)
	{
		if (m_ppMeshes[nIndex]) m_ppMeshes[nIndex]->Release();
		m_ppMeshes[nIndex] = pMesh;
		if (pMesh) pMesh->AddRef();
	}
}

XMFLOAT2 CHeightMapTerrain::GetPipelineMode()
{
	return XMFLOAT2((*m_pxmf2TessFactor).x, m_bPipelineStateIndex);
}

void CHeightMapTerrain::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = sizeof(float) * 2; //256의 배수
	m_pd3dcbTessFactor = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbTessFactor->Map(0, NULL, (void**)&m_pxmf2TessFactor);

	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CHeightMapTerrain::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	D3D12_GPU_VIRTUAL_ADDRESS d3dcbGpuVirtualAddress = m_pd3dcbTessFactor->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(9, d3dcbGpuVirtualAddress);

}

void CHeightMapTerrain::SetTessellationMode(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if ((*m_pxmf2TessFactor).x > 20)
	{
		*m_pxmf2TessFactor = XMFLOAT2(2, 2);
	}
	else
	{
		(*m_pxmf2TessFactor).x += 2;
		(*m_pxmf2TessFactor).y += 2;
	}

}

void CHeightMapTerrain::ChangePipeLine()
{
	m_bPipelineStateIndex = (!m_bPipelineStateIndex);
}

void CHeightMapTerrain::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();


	m_ppMaterials[0]->m_pShader->Render(pd3dCommandList, pCamera);
	m_ppMaterials[0]->UpdateShaderVariable(pd3dCommandList);
	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
	UpdateShaderVariables(pd3dCommandList);

	//pd3dCommandList->SetGraphicsRootDescriptorTable(2, m_d3dCbvGPUDescriptorHandle);

	if (m_ppMeshes)
	{
		for (int i = 0; i < m_nMeshes; i++)
		{
			if (m_ppMeshes[i]) m_ppMeshes[i]->Render(pd3dCommandList, i);
		}
	}
}