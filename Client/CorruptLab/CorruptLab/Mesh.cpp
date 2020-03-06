//-----------------------------------------------------------------------------
// File: CMesh.cpp
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include "Mesh.h"
#include "Object.h"

// CMesh-----------------------------------------------------------------------------------------
CMesh::CMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

CMesh::~CMesh()
{
	if (m_MeshInfo.m_pd3dPositionBuffer)  m_MeshInfo.m_pd3dPositionBuffer->Release();
	if (m_MeshInfo.m_pxmf3Positions) delete m_MeshInfo.m_pxmf3Positions;

	if (m_MeshInfo.m_pd3dColorBuffer) m_MeshInfo.m_pd3dColorBuffer->Release();
	if (m_MeshInfo.m_pxmf4Colors) delete m_MeshInfo.m_pxmf4Colors;

	if (m_MeshInfo.m_pd3dTextureCoord0Buffer) m_MeshInfo.m_pd3dTextureCoord0Buffer->Release();
	if (m_MeshInfo.m_pxmf2TextureCoords0) delete m_MeshInfo.m_pxmf2TextureCoords0;

	if (m_MeshInfo.m_pd3dTextureCoord1Buffer) m_MeshInfo.m_pd3dTextureCoord1Buffer->Release();
	if (m_MeshInfo.m_pxmf2TextureCoords1) delete m_MeshInfo.m_pxmf2TextureCoords1;


	if (m_MeshInfo.m_nSubMeshes > 0)
	{
		for (int i = 0; i < m_MeshInfo.m_nSubMeshes; ++i)
		{
			if (m_MeshInfo.m_ppd3dSubSetIndexBuffers[i])
				m_MeshInfo.m_ppd3dSubSetIndexBuffers[i]->Release();
			if (m_MeshInfo.m_ppnSubSetIndices[i]) delete[] m_MeshInfo.m_ppnSubSetIndices[i];
		}
		if (m_MeshInfo.m_ppd3dSubSetIndexBuffers) delete[] m_MeshInfo.m_ppd3dSubSetIndexBuffers;
		if (m_MeshInfo.m_pd3dSubSetIndexBufferViews) delete[] m_MeshInfo.m_pd3dSubSetIndexBufferViews;

		if (m_MeshInfo.m_pnSubSetIndices) delete[]  m_MeshInfo.m_pnSubSetIndices;
		if (m_MeshInfo.m_ppnSubSetIndices) delete[] m_MeshInfo.m_ppnSubSetIndices;
	}
}

void CMesh::ReleaseUploadBuffers()
{
	if (m_MeshInfo.m_pd3dPositionUploadBuffer) m_MeshInfo.m_pd3dPositionUploadBuffer->Release();
	if (m_MeshInfo.m_pd3dColorUploadBuffer) m_MeshInfo.m_pd3dColorUploadBuffer->Release();
	if (m_MeshInfo.m_pd3dTextureCoord0UploadBuffer) m_MeshInfo.m_pd3dTextureCoord0UploadBuffer->Release();
	if (m_MeshInfo.m_pd3dTextureCoord1UploadBuffer) m_MeshInfo.m_pd3dTextureCoord1UploadBuffer->Release();

	m_MeshInfo.m_pd3dPositionUploadBuffer = NULL;
	m_MeshInfo.m_pd3dColorUploadBuffer = NULL;
	m_MeshInfo.m_pd3dTextureCoord0UploadBuffer = NULL;
	m_MeshInfo.m_pd3dTextureCoord1UploadBuffer = NULL;

	if ((m_MeshInfo.m_nSubMeshes > 0) && m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers)
	{
		for (int i = 0; i < m_MeshInfo.m_nSubMeshes; i++)
		{
			if (m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers[i])
				m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers[i]->Release();
		}
		if (m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers)
			delete[] m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers;
		m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers = NULL;
	}
};

// CStandardMesh----------------------------------------------------------------------------------
CStandardMesh::CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CMesh(pd3dDevice, pd3dCommandList)
{
}

CStandardMesh::~CStandardMesh()
{
	if (m_pd3dNormalBuffer)m_pd3dNormalBuffer->Release();
	if (m_pxmf3Normals) delete m_pxmf3Normals;

	if (m_pd3dTangentBuffer)m_pd3dTangentBuffer->Release();
	if (m_pxmf3Tangents) delete m_pxmf3Tangents;

	if (m_pd3dBiTangentBuffer)m_pd3dBiTangentBuffer->Release();
	if (m_pxmf3BiTangents) delete m_pxmf3BiTangents;
}

void CStandardMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();

	if (m_pd3dNormalUploadBuffer) m_pd3dNormalUploadBuffer->Release();
	m_pd3dNormalUploadBuffer = NULL;

	if (m_pd3dTangentUploadBuffer) m_pd3dTangentUploadBuffer->Release();
	m_pd3dTangentUploadBuffer = NULL;

	if (m_pd3dBiTangentUploadBuffer) m_pd3dBiTangentUploadBuffer->Release();
	m_pd3dBiTangentUploadBuffer = NULL;
}

void CStandardMesh::LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	UINT nReads = 0;

	int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0,
		nSubMeshes = 0, nSubIndices = 0, nTangents = 0, nBiTangents = 0, nTextureCoords = 0;

	m_nVertices = ::ReadIntegerFromFile(pInFile);

	::ReadStringFromFile(pInFile, m_pstrMeshName);

	for (; ; )
	{
		::ReadStringFromFile(pInFile, pstrToken);

		if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&(m_xmf3AABBCenter), sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&(m_xmf3AABBExtents), sizeof(XMFLOAT3), 1, pInFile);
		}

		else if (!strcmp(pstrToken, "<Positions>:"))
		{
			nPositions = ::ReadIntegerFromFile(pInFile);
			if (nPositions > 0)
			{
				m_nType |= VERTEXT_POSITION;
				m_MeshInfo.m_pxmf3Positions = new XMFLOAT3[nPositions];
				nReads = (UINT)::fread(m_MeshInfo.m_pxmf3Positions, sizeof(XMFLOAT3), nPositions, pInFile);

				m_MeshInfo.m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf3Positions,
					sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dPositionUploadBuffer);

				m_MeshInfo.m_d3dPositionBufferView.BufferLocation = m_MeshInfo.m_pd3dPositionBuffer->GetGPUVirtualAddress();
				m_MeshInfo.m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_MeshInfo.m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}

		else if (!strcmp(pstrToken, "<Colors>:"))
		{
			nColors = ::ReadIntegerFromFile(pInFile);
			if (nColors > 0)
			{
				m_nType |= VERTEXT_COLOR;
				m_MeshInfo.m_pxmf4Colors = new XMFLOAT4[nColors];
				nReads = (UINT)::fread(m_MeshInfo.m_pxmf4Colors, sizeof(XMFLOAT4), nColors, pInFile);
			}
		}

		else if (!strcmp(pstrToken, "<Normals>:"))
		{
			nNormals = ::ReadIntegerFromFile(pInFile);
			if (nNormals > 0)
			{
				m_nType |= VERTEXT_NORMAL;
				m_pxmf3Normals = new XMFLOAT3[nNormals];
				nReads = (UINT)::fread(m_pxmf3Normals, sizeof(XMFLOAT3), nNormals, pInFile);

				m_pd3dNormalBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Normals,
					sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dNormalUploadBuffer);

				m_d3dNormalBufferView.BufferLocation = m_pd3dNormalBuffer->GetGPUVirtualAddress();
				m_d3dNormalBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dNormalBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}

		else if (!strcmp(pstrToken, "<Tangents>:"))
		{
			nReads = (UINT)::fread(&nTangents, sizeof(int), 1, pInFile);
			if (nTangents > 0)
			{
				m_nType |= VERTEXT_TANGENT;
				m_pxmf3Tangents = new XMFLOAT3[nTangents];
				nReads = (UINT)::fread(m_pxmf3Tangents, sizeof(XMFLOAT3), nTangents, pInFile);

				m_pd3dTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3Tangents,
					sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dTangentUploadBuffer);

				m_d3dTangentBufferView.BufferLocation = m_pd3dTangentBuffer->GetGPUVirtualAddress();
				m_d3dTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}

		else if (!strcmp(pstrToken, "<BiTangents>:"))
		{
			nReads = (UINT)::fread(&nBiTangents, sizeof(int), 1, pInFile);
			if (nBiTangents > 0)
			{
				m_pxmf3BiTangents = new XMFLOAT3[nBiTangents];
				nReads = (UINT)::fread(m_pxmf3BiTangents, sizeof(XMFLOAT3), nBiTangents, pInFile);

				m_pd3dBiTangentBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf3BiTangents,
					sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBiTangentUploadBuffer);

				m_d3dBiTangentBufferView.BufferLocation = m_pd3dBiTangentBuffer->GetGPUVirtualAddress();
				m_d3dBiTangentBufferView.StrideInBytes = sizeof(XMFLOAT3);
				m_d3dBiTangentBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
			}
		}

		else if (!strcmp(pstrToken, "<TextureCoords0>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD0;
				m_MeshInfo.m_pxmf2TextureCoords0 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_MeshInfo.m_pxmf2TextureCoords0, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_MeshInfo.m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf2TextureCoords0,
					sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dTextureCoord0UploadBuffer);

				m_MeshInfo.m_d3dTextureCoord0BufferView.BufferLocation = m_MeshInfo.m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
				m_MeshInfo.m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_MeshInfo.m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}
		//TexCoords
		else if (!strcmp(pstrToken, "<TextureCoords1>:"))
		{
			nReads = (UINT)::fread(&nTextureCoords, sizeof(int), 1, pInFile);
			if (nTextureCoords > 0)
			{
				m_nType |= VERTEXT_TEXTURE_COORD1;
				m_MeshInfo.m_pxmf2TextureCoords1 = new XMFLOAT2[nTextureCoords];
				nReads = (UINT)::fread(m_MeshInfo.m_pxmf2TextureCoords1, sizeof(XMFLOAT2), nTextureCoords, pInFile);

				m_MeshInfo.m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf2TextureCoords1,
					sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dTextureCoord1UploadBuffer);

				m_MeshInfo.m_d3dTextureCoord1BufferView.BufferLocation = m_MeshInfo.m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
				m_MeshInfo.m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
				m_MeshInfo.m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
			}
		}

		else if (!strcmp(pstrToken, "<SubMeshes>:"))
		{
			m_MeshInfo.m_nSubMeshes = ::ReadIntegerFromFile(pInFile);
			if (m_MeshInfo.m_nSubMeshes > 0)
			{
				m_MeshInfo.m_pnSubSetIndices = new int[m_MeshInfo.m_nSubMeshes];
				m_MeshInfo.m_ppnSubSetIndices = new UINT * [m_MeshInfo.m_nSubMeshes];

				m_MeshInfo.m_ppd3dSubSetIndexBuffers = new ID3D12Resource * [m_MeshInfo.m_nSubMeshes];
				m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers = new ID3D12Resource * [m_MeshInfo.m_nSubMeshes];
				m_MeshInfo.m_pd3dSubSetIndexBufferViews = new D3D12_INDEX_BUFFER_VIEW[m_MeshInfo.m_nSubMeshes];

				for (int i = 0; i < m_MeshInfo.m_nSubMeshes; i++)
				{
					::ReadStringFromFile(pInFile, pstrToken);

					if (!strcmp(pstrToken, "<SubMesh>:"))
					{
						int nIndex = 0;
						nReads = (UINT)::fread(&nIndex, sizeof(int), 1, pInFile); //i
						nReads = (UINT)::fread(&(m_MeshInfo.m_pnSubSetIndices[i]), sizeof(int), 1, pInFile);

						if (m_MeshInfo.m_pnSubSetIndices[i] > 0)
						{
							m_MeshInfo.m_ppnSubSetIndices[i] = new UINT[m_MeshInfo.m_pnSubSetIndices[i]];
							nReads = (UINT)::fread(m_MeshInfo.m_ppnSubSetIndices[i], sizeof(UINT), m_MeshInfo.m_pnSubSetIndices[i], pInFile);

							m_MeshInfo.m_ppd3dSubSetIndexBuffers[i] = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_ppnSubSetIndices[i],
								sizeof(UINT) * m_MeshInfo.m_pnSubSetIndices[i], D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_INDEX_BUFFER, &m_MeshInfo.m_ppd3dSubSetIndexUploadBuffers[i]);

							m_MeshInfo.m_pd3dSubSetIndexBufferViews[i].BufferLocation = m_MeshInfo.m_ppd3dSubSetIndexBuffers[i]->GetGPUVirtualAddress();
							m_MeshInfo.m_pd3dSubSetIndexBufferViews[i].Format = DXGI_FORMAT_R32_UINT;
							m_MeshInfo.m_pd3dSubSetIndexBufferViews[i].SizeInBytes = sizeof(UINT) * m_MeshInfo.m_pnSubSetIndices[i];
						}
					}
				}
			}
		}
		else if (!strcmp(pstrToken, "</Mesh>"))
		{
			break;
		}
	}
}

void CStandardMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	const int ViewNum = 5;
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[ViewNum] =
	{ m_MeshInfo.m_d3dPositionBufferView, m_MeshInfo.m_d3dTextureCoord0BufferView, m_d3dNormalBufferView, m_d3dTangentBufferView, m_d3dBiTangentBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, ViewNum, pVertexBufferViews);

	if ((m_MeshInfo.m_nSubMeshes > 0) && (nSubSet < m_MeshInfo.m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_MeshInfo.m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_MeshInfo.m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

// CSkinnedMesh-----------------------------------------------------------------------------------
CSkinnedMesh::CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) : CStandardMesh(pd3dDevice, pd3dCommandList) {}

CSkinnedMesh::~CSkinnedMesh()
{
	if (m_pd3dBoneIndexBuffer) m_pd3dBoneIndexBuffer->Release();
	if (m_pxmu4BoneIndices) delete m_pxmu4BoneIndices;

	if (m_pd3dBoneWeightBuffer) m_pd3dBoneWeightBuffer->Release();
	if (m_pxmf4BoneWeights) delete m_pxmf4BoneWeights;

	if (m_ppSkinningBoneFrameCaches) delete[] m_ppSkinningBoneFrameCaches;
	if (m_ppstrSkinningBoneNames) delete[] m_ppstrSkinningBoneNames;

	if (m_pxmf4x4BindPoseBoneOffsets) delete[] m_pxmf4x4BindPoseBoneOffsets;

	ReleaseShaderVariables();
}

void CSkinnedMesh::LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile)
{
	char pstrToken[64] = { '\0' };
	BYTE nStrLength = 0;

	UINT nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
	nReads = (UINT)::fread(m_pstrSkinnedMeshName, sizeof(char), nStrLength, pInFile);
	m_pstrSkinnedMeshName[nStrLength] = '\0';

	for (; ; )
	{
		nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
		nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pInFile);
		pstrToken[nStrLength] = '\0';

		if (!strcmp(pstrToken, "<BonesPerVertex>:"))
		{
			nReads = (UINT)::fread(&m_nBonesPerVertex, sizeof(int), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<Bounds>:"))
		{
			nReads = (UINT)::fread(&m_xmf3AABBCenter, sizeof(XMFLOAT3), 1, pInFile);
			nReads = (UINT)::fread(&m_xmf3AABBExtents, sizeof(XMFLOAT3), 1, pInFile);
		}
		else if (!strcmp(pstrToken, "<BoneNames>:"))
		{
			nReads = (UINT)::fread(&m_nSkinningBones, sizeof(int), 1, pInFile);
			if (m_nSkinningBones > 0)
			{
				m_ppstrSkinningBoneNames = new char[m_nSkinningBones][64];
				m_ppSkinningBoneFrameCaches = new CGameObject * [m_nSkinningBones];
				for (int i = 0; i < m_nSkinningBones; i++)
				{
					nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pInFile);
					nReads = (UINT)::fread(m_ppstrSkinningBoneNames[i], sizeof(char), nStrLength, pInFile);
					m_ppstrSkinningBoneNames[i][nStrLength] = '\0';

					m_ppSkinningBoneFrameCaches[i] = NULL;
				}
			}
		}
		else if (!strcmp(pstrToken, "<BoneOffsets>:"))
		{
			nReads = (UINT)::fread(&m_nSkinningBones, sizeof(int), 1, pInFile);
			if (m_nSkinningBones > 0)
			{
				m_pxmf4x4BindPoseBoneOffsets = new XMFLOAT4X4[m_nSkinningBones];
				nReads = (UINT)::fread(m_pxmf4x4BindPoseBoneOffsets, sizeof(float), 16 * m_nSkinningBones, pInFile);
			}
		}
		else if (!strcmp(pstrToken, "<BoneWeights>:"))
		{
			m_nType |= VERTEXT_BONE_INDEX_WEIGHT;

			nReads = (UINT)::fread(&m_nVertices, sizeof(int), 1, pInFile);
			if (m_nVertices > 0)
			{
				m_pxmu4BoneIndices = new XMUINT4[m_nVertices];
				m_pxmf4BoneWeights = new XMFLOAT4[m_nVertices];

				nReads = (UINT)::fread(m_pxmu4BoneIndices, sizeof(XMUINT4), m_nVertices, pInFile);
				m_pd3dBoneIndexBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmu4BoneIndices,
					sizeof(XMUINT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneIndexUploadBuffer);

				m_d3dBoneIndexBufferView.BufferLocation = m_pd3dBoneIndexBuffer->GetGPUVirtualAddress();
				m_d3dBoneIndexBufferView.StrideInBytes = sizeof(XMUINT4);
				m_d3dBoneIndexBufferView.SizeInBytes = sizeof(XMUINT4) * m_nVertices;

				nReads = (UINT)::fread(m_pxmf4BoneWeights, sizeof(XMFLOAT4), m_nVertices, pInFile);
				m_pd3dBoneWeightBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pxmf4BoneWeights,
					sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dBoneWeightUploadBuffer);

				m_d3dBoneWeightBufferView.BufferLocation = m_pd3dBoneWeightBuffer->GetGPUVirtualAddress();
				m_d3dBoneWeightBufferView.StrideInBytes = sizeof(XMFLOAT4);
				m_d3dBoneWeightBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;
			}
		}
		else if (!strcmp(pstrToken, "</SkinningInfo>"))
		{
			break;
		}
	}
}

void CSkinnedMesh::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = (((sizeof(XMFLOAT4X4) * SKINNED_ANIMATION_BONES) + 255) & ~255); //256�� ���
	m_pd3dcbBoneOffsets = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbBoneOffsets->Map(0, NULL, (void**)&m_pcbxmf4x4BoneOffsets);

	m_pd3dcbBoneTransforms = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);
	m_pd3dcbBoneTransforms->Map(0, NULL, (void**)&m_pcbxmf4x4BoneTransforms);
}

void CSkinnedMesh::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	if (m_pd3dcbBoneOffsets && m_pd3dcbBoneTransforms)
	{
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneOffsetsGpuVirtualAddress = m_pd3dcbBoneOffsets->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(11, d3dcbBoneOffsetsGpuVirtualAddress); //Skinned Bone Offsets
		D3D12_GPU_VIRTUAL_ADDRESS d3dcbBoneTransformsGpuVirtualAddress = m_pd3dcbBoneTransforms->GetGPUVirtualAddress();
		pd3dCommandList->SetGraphicsRootConstantBufferView(10, d3dcbBoneTransformsGpuVirtualAddress); //Skinned Bone Transforms

		for (int i = 0; i < m_nSkinningBones; i++)
		{
			XMStoreFloat4x4(&m_pcbxmf4x4BoneOffsets[i], XMMatrixTranspose(XMLoadFloat4x4(&m_pxmf4x4BindPoseBoneOffsets[i])));
			XMStoreFloat4x4(&m_pcbxmf4x4BoneTransforms[i], XMMatrixTranspose(XMLoadFloat4x4(&m_ppSkinningBoneFrameCaches[i]->m_xmf4x4World)));
		}
	}
}

void CSkinnedMesh::ReleaseShaderVariables()
{
	if (m_pd3dcbBoneOffsets) m_pd3dcbBoneOffsets->Release();
	if (m_pd3dcbBoneTransforms) m_pd3dcbBoneTransforms->Release();
}

void CSkinnedMesh::ReleaseUploadBuffers()
{
	CStandardMesh::ReleaseUploadBuffers();

	if (m_pd3dBoneIndexUploadBuffer) m_pd3dBoneIndexUploadBuffer->Release();
	m_pd3dBoneIndexUploadBuffer = NULL;

	if (m_pd3dBoneWeightUploadBuffer) m_pd3dBoneWeightUploadBuffer->Release();
	m_pd3dBoneWeightUploadBuffer = NULL;
}

void CSkinnedMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	UpdateShaderVariables(pd3dCommandList);

	const int ViewNum = 7;
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[ViewNum] =
	{ m_MeshInfo.m_d3dPositionBufferView, m_MeshInfo.m_d3dTextureCoord0BufferView, m_d3dNormalBufferView,
		m_d3dTangentBufferView, m_d3dBiTangentBufferView, m_d3dBoneIndexBufferView,m_d3dBoneWeightBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, ViewNum, pVertexBufferViews);

	if ((m_MeshInfo.m_nSubMeshes > 0) && (nSubSet < m_MeshInfo.m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_MeshInfo.m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_MeshInfo.m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

void CSkinnedMesh::PrepareSkinning(CGameObject* pModelRootObject)
{
}

// CHeightMapImage-----------------------------------------------------------------------------------
CHeightMapImage::CHeightMapImage(LPCTSTR pFileName, int nWidth, int nLength, XMFLOAT3 xmf3Scale)
{
	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	BYTE* pHeightMapPixels = new BYTE[m_nWidth * m_nLength];

	HANDLE hFile = ::CreateFile(pFileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_READONLY, NULL);
	DWORD dwBytesRead;
	::ReadFile(hFile, pHeightMapPixels, (m_nWidth * m_nLength), &dwBytesRead, NULL);
	::CloseHandle(hFile);

	m_pHeightMapPixels = new BYTE[m_nWidth * m_nLength];
	for (int y = 0; y < m_nLength; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			m_pHeightMapPixels[x + ((m_nLength - 1 - y) * m_nWidth)] = pHeightMapPixels[x + (y * m_nWidth)];
		}
	}

	if (pHeightMapPixels) delete[] pHeightMapPixels;
}

CHeightMapImage::~CHeightMapImage()
{
	if (m_pHeightMapPixels)
		delete[] m_pHeightMapPixels;
	m_pHeightMapPixels = NULL;
}

XMFLOAT3 CHeightMapImage::GetHeightMapNormal(int x, int z)
{
	if ((x < 0.0f) || (z < 0.0f) || (x >= m_nWidth) || (z >= m_nLength)) return(XMFLOAT3(0.0f, 1.0f, 0.0f));

	int nHeightMapIndex = x + (z * m_nWidth);
	int xHeightMapAdd = (x < (m_nWidth - 1)) ? 1 : -1;
	int zHeightMapAdd = (z < (m_nLength - 1)) ? m_nWidth : -m_nWidth;
	float y1 = (float)m_pHeightMapPixels[nHeightMapIndex] * m_xmf3Scale.y;
	float y2 = (float)m_pHeightMapPixels[nHeightMapIndex + xHeightMapAdd] * m_xmf3Scale.y;
	float y3 = (float)m_pHeightMapPixels[nHeightMapIndex + zHeightMapAdd] * m_xmf3Scale.y;
	XMFLOAT3 xmf3Edge1 = XMFLOAT3(0.0f, y3 - y1, m_xmf3Scale.z);
	XMFLOAT3 xmf3Edge2 = XMFLOAT3(m_xmf3Scale.x, y2 - y1, 0.0f);
	XMFLOAT3 xmf3Normal = Vector3::CrossProduct(xmf3Edge1, xmf3Edge2, true);

	return(xmf3Normal);
}

#define _WITH_APPROXIMATE_OPPOSITE_CORNER

float CHeightMapImage::GetHeight(float fx, float fz, bool bReverseQuad)
{
	fx = fx / m_xmf3Scale.x;
	fz = fz / m_xmf3Scale.z;
	if ((fx < 0.0f) || (fz < 0.0f) || (fx >= m_nWidth) || (fz >= m_nLength)) return(0.0f);

	int x = (int)fx;
	int z = (int)fz;
	float fxPercent = fx - x;
	float fzPercent = fz - z;

	float fBottomLeft = (float)m_pHeightMapPixels[x + (z * m_nWidth)];
	float fBottomRight = (float)m_pHeightMapPixels[(x + 1) + (z * m_nWidth)];
	float fTopLeft = (float)m_pHeightMapPixels[x + ((z + 1) * m_nWidth)];
	float fTopRight = (float)m_pHeightMapPixels[(x + 1) + ((z + 1) * m_nWidth)];
#ifdef _WITH_APPROXIMATE_OPPOSITE_CORNER
	if (bReverseQuad)
	{
		if (fzPercent >= fxPercent)
			fBottomRight = fBottomLeft + (fTopRight - fTopLeft);
		else
			fTopLeft = fTopRight + (fBottomLeft - fBottomRight);
	}
	else
	{
		if (fzPercent < (1.0f - fxPercent))
			fTopRight = fTopLeft + (fBottomRight - fBottomLeft);
		else
			fBottomLeft = fTopLeft + (fBottomRight - fTopRight);
	}
#endif
	float fTopHeight = fTopLeft * (1 - fxPercent) + fTopRight * fxPercent;
	float fBottomHeight = fBottomLeft * (1 - fxPercent) + fBottomRight * fxPercent;
	float fHeight = fBottomHeight * (1 - fzPercent) + fTopHeight * fzPercent;

	return(fHeight);
}

// CHeightMapGridMesh-----------------------------------------------------------------------------------
CHeightMapGridMesh::CHeightMapGridMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
	int xStart, int zStart, int nWidth, int nLength, XMFLOAT3 xmf3Scale, XMFLOAT4 xmf4Color, void* pContext)
{
	m_nVertices = 25;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;

	m_nWidth = nWidth;
	m_nLength = nLength;
	m_xmf3Scale = xmf3Scale;

	m_MeshInfo.m_pxmf3Positions = new XMFLOAT3[m_nVertices];
	m_MeshInfo.m_pxmf4Colors = new XMFLOAT4[m_nVertices];
	m_MeshInfo.m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices];
	m_MeshInfo.m_pxmf2TextureCoords1 = new XMFLOAT2[m_nVertices];

	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	int cxHeightMap = pHeightMapImage->GetHeightMapWidth();
	int czHeightMap = pHeightMapImage->GetHeightMapLength();

	float fHeight = 0.0f, fMinHeight = +FLT_MAX, fMaxHeight = -FLT_MAX;
	for (int i = 0, z = (zStart + nLength - 1); z >= zStart; z -= 2)
	{
		for (int x = xStart; x < (xStart + nWidth); x += 2, i++)
		{
			fHeight = OnGetHeight(x, z, pContext);
			m_MeshInfo.m_pxmf3Positions[i] = XMFLOAT3((x * m_xmf3Scale.x), fHeight, (z * m_xmf3Scale.z));
			m_MeshInfo.m_pxmf4Colors[i] = Vector4::Add(OnGetColor(x, z, pContext), xmf4Color);
			m_MeshInfo.m_pxmf2TextureCoords0[i] = XMFLOAT2(float(x) / float(cxHeightMap - 1), float(czHeightMap - 1 - z) / float(czHeightMap - 1));
			m_MeshInfo.m_pxmf2TextureCoords1[i] = XMFLOAT2(float(x) / float(m_xmf3Scale.x * 0.5f), float(z) / float(m_xmf3Scale.z * 0.5f));

			if (fHeight < fMinHeight) fMinHeight = fHeight;
			if (fHeight > fMaxHeight) fMaxHeight = fHeight;
		}
	}

	m_MeshInfo.m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dPositionUploadBuffer);

	m_MeshInfo.m_d3dPositionBufferView.BufferLocation = m_MeshInfo.m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_MeshInfo.m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_MeshInfo.m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;

	m_MeshInfo.m_pd3dColorBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf4Colors, sizeof(XMFLOAT4) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dColorUploadBuffer);

	m_MeshInfo.m_d3dColorBufferView.BufferLocation = m_MeshInfo.m_pd3dColorBuffer->GetGPUVirtualAddress();
	m_MeshInfo.m_d3dColorBufferView.StrideInBytes = sizeof(XMFLOAT4);
	m_MeshInfo.m_d3dColorBufferView.SizeInBytes = sizeof(XMFLOAT4) * m_nVertices;


	m_MeshInfo.m_pd3dTextureCoord0Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf2TextureCoords0, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dTextureCoord0UploadBuffer);

	m_MeshInfo.m_d3dTextureCoord0BufferView.BufferLocation = m_MeshInfo.m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
	m_MeshInfo.m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_MeshInfo.m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;

	m_MeshInfo.m_pd3dTextureCoord1Buffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf2TextureCoords1, sizeof(XMFLOAT2) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dTextureCoord1UploadBuffer);

	m_MeshInfo.m_d3dTextureCoord1BufferView.BufferLocation = m_MeshInfo.m_pd3dTextureCoord1Buffer->GetGPUVirtualAddress();
	m_MeshInfo.m_d3dTextureCoord1BufferView.StrideInBytes = sizeof(XMFLOAT2);
	m_MeshInfo.m_d3dTextureCoord1BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
}

CHeightMapGridMesh::~CHeightMapGridMesh() {}

float CHeightMapGridMesh::OnGetHeight(int x, int z, void* pContext)
{
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	BYTE* pHeightMapPixels = pHeightMapImage->GetHeightMapPixels();
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	int nWidth = pHeightMapImage->GetHeightMapWidth();
	float fHeight = pHeightMapPixels[x + (z * nWidth)] * xmf3Scale.y;
	return(fHeight);
}

XMFLOAT4 CHeightMapGridMesh::OnGetColor(int x, int z, void* pContext)
{
	XMFLOAT3 xmf3LightDirection = XMFLOAT3(-1.0f, 1.0f, 1.0f);
	xmf3LightDirection = Vector3::Normalize(xmf3LightDirection);
	CHeightMapImage* pHeightMapImage = (CHeightMapImage*)pContext;
	XMFLOAT3 xmf3Scale = pHeightMapImage->GetScale();
	XMFLOAT4 xmf4IncidentLightColor(0.6f, 0.5f, 0.2f, 1.0f);
	float fScale = Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x + 1, z + 1), xmf3LightDirection);
	fScale += Vector3::DotProduct(pHeightMapImage->GetHeightMapNormal(x, z + 1), xmf3LightDirection);
	fScale = (fScale / 4.0f) + 0.05f;
	if (fScale > 1.0f) fScale = 1.0f;
	if (fScale < 0.25f) fScale = 0.25f;
	XMFLOAT4 xmf4Color = Vector4::Multiply(fScale, xmf4IncidentLightColor);
	return(xmf4Color);
}

void CHeightMapGridMesh::ReleaseUploadBuffers()
{
	CMesh::ReleaseUploadBuffers();
}

void CHeightMapGridMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[4] = { m_MeshInfo.m_d3dPositionBufferView,  m_MeshInfo.m_d3dColorBufferView, m_MeshInfo.m_d3dTextureCoord0BufferView, m_MeshInfo.m_d3dTextureCoord1BufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 4, pVertexBufferViews);

	if ((m_MeshInfo.m_nSubMeshes > 0) && (nSubSet < m_MeshInfo.m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_MeshInfo.m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_MeshInfo.m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

// CSkyBoxMesh-----------------------------------------------------------------------------------
CSkyBoxMesh::CSkyBoxMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth) : CMesh(pd3dDevice, pd3dCommandList)
{
	m_nVertices = 36;
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	m_MeshInfo.m_pxmf3Positions = new XMFLOAT3[m_nVertices];

	float fx = fWidth * 0.5f, fy = fHeight * 0.5f, fz = fDepth * 0.5f;
	// Front Quad (quads point inward)
	m_MeshInfo.m_pxmf3Positions[0] = XMFLOAT3(-fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[1] = XMFLOAT3(+fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[2] = XMFLOAT3(-fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[3] = XMFLOAT3(-fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[4] = XMFLOAT3(+fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[5] = XMFLOAT3(+fx, -fx, +fx);
	// Back Quad										
	m_MeshInfo.m_pxmf3Positions[6] = XMFLOAT3(+fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[7] = XMFLOAT3(-fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[8] = XMFLOAT3(+fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[9] = XMFLOAT3(+fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[10] = XMFLOAT3(-fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[11] = XMFLOAT3(-fx, -fx, -fx);
	// Left Quad										
	m_MeshInfo.m_pxmf3Positions[12] = XMFLOAT3(-fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[13] = XMFLOAT3(-fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[14] = XMFLOAT3(-fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[15] = XMFLOAT3(-fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[16] = XMFLOAT3(-fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[17] = XMFLOAT3(-fx, -fx, +fx);
	// Right Quad										
	m_MeshInfo.m_pxmf3Positions[18] = XMFLOAT3(+fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[19] = XMFLOAT3(+fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[20] = XMFLOAT3(+fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[21] = XMFLOAT3(+fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[22] = XMFLOAT3(+fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[23] = XMFLOAT3(+fx, -fx, -fx);
	// Top Quad											
	m_MeshInfo.m_pxmf3Positions[24] = XMFLOAT3(-fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[25] = XMFLOAT3(+fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[26] = XMFLOAT3(-fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[27] = XMFLOAT3(-fx, +fx, +fx);
	m_MeshInfo.m_pxmf3Positions[28] = XMFLOAT3(+fx, +fx, -fx);
	m_MeshInfo.m_pxmf3Positions[29] = XMFLOAT3(+fx, +fx, +fx);
	// Bottom Quad										
	m_MeshInfo.m_pxmf3Positions[30] = XMFLOAT3(-fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[31] = XMFLOAT3(+fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[32] = XMFLOAT3(-fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[33] = XMFLOAT3(-fx, -fx, -fx);
	m_MeshInfo.m_pxmf3Positions[34] = XMFLOAT3(+fx, -fx, +fx);
	m_MeshInfo.m_pxmf3Positions[35] = XMFLOAT3(+fx, -fx, -fx);

	m_MeshInfo.m_pd3dPositionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dPositionUploadBuffer);

	m_MeshInfo.m_d3dPositionBufferView.BufferLocation = m_MeshInfo.m_pd3dPositionBuffer->GetGPUVirtualAddress();
	m_MeshInfo.m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
	m_MeshInfo.m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
}

CSkyBoxMesh::~CSkyBoxMesh()
{
	//CMesh::~CMesh();
	CMesh::ReleaseUploadBuffers();
}

void CSkyBoxMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[1] = { m_MeshInfo.m_d3dPositionBufferView };
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, pVertexBufferViews);

	if ((m_MeshInfo.m_nSubMeshes > 0) && (nSubSet < m_MeshInfo.m_nSubMeshes))
	{
		pd3dCommandList->IASetIndexBuffer(&(m_MeshInfo.m_pd3dSubSetIndexBufferViews[nSubSet]));
		pd3dCommandList->DrawIndexedInstanced(m_MeshInfo.m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
	}
	else
	{
		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
	}
}

//// BillboadRectMesh ====================================================================================
//CBillboardRectMesh::CBillboardRectMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth, float fxPosition, float fyPosition, float fzPosition)
//{
//	m_nVertices = 6;
//
//	m_MeshInfo.m_pxmf3Positions = new XMFLOAT3[m_nVertices];
//	m_MeshInfo.m_pxmf2TextureCoords0 = new XMFLOAT2[m_nVertices]; 
//
//	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
//
//	float fx = (fWidth * 0.5f) + fxPosition;
//	float fy = (fHeight * 0.5f) + fyPosition;
//	float fz = (fDepth * 0.5f) + fzPosition;
//
//	if (fDepth == 0.0f)
//	{
//		if (fzPosition > 0.0f)
//		{
//			m_MeshInfo.m_pxmf3Positions[0] = XMFLOAT3(+fx, +fy, fz);
//			m_MeshInfo.m_pxmf3Positions[1] = XMFLOAT3(+fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[2] = XMFLOAT3(-fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[3] = XMFLOAT3(-fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[4] = XMFLOAT3(-fx, +fy, fz);
//			m_MeshInfo.m_pxmf3Positions[5] = XMFLOAT3(+fx, +fy, fz);
//
//			m_MeshInfo.m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
//		}
//		else
//		{
//			m_MeshInfo.m_pxmf3Positions[0] = XMFLOAT3(-fx, +fy, fz);
//			m_MeshInfo.m_pxmf3Positions[1] = XMFLOAT3(-fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[2] = XMFLOAT3(+fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[3] = XMFLOAT3(+fx, -fy, fz);
//			m_MeshInfo.m_pxmf3Positions[4] = XMFLOAT3(+fx, +fy, fz);
//			m_MeshInfo.m_pxmf3Positions[5] = XMFLOAT3(-fx, +fy, fz);
//
//			m_MeshInfo.m_pxmf2TextureCoords0[0] = XMFLOAT2(1.0f, 0.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[1] = XMFLOAT2(1.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[2] = XMFLOAT2(0.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[3] = XMFLOAT2(0.0f, 1.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[4] = XMFLOAT2(0.0f, 0.0f);
//			m_MeshInfo.m_pxmf2TextureCoords0[5] = XMFLOAT2(1.0f, 0.0f);
//		}
//	}
//
//	m_MeshInfo.m_pd3dPositionBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf3Positions, sizeof(XMFLOAT3) * m_nVertices,
//  D3D12_HEAP_TYPE_DEFAULT,D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dPositionUploadBuffer);
//	m_MeshInfo.m_d3dPositionBufferView.BufferLocation = m_MeshInfo.m_pd3dPositionBuffer->GetGPUVirtualAddress();
//	m_MeshInfo.m_d3dPositionBufferView.StrideInBytes = sizeof(XMFLOAT3);
//	m_MeshInfo.m_d3dPositionBufferView.SizeInBytes = sizeof(XMFLOAT3) * m_nVertices;
//
//	m_MeshInfo.m_pd3dTextureCoord0Buffer = CreateBufferResource(pd3dDevice, pd3dCommandList, m_MeshInfo.m_pxmf3Positions, sizeof(XMFLOAT2) * m_nVertices,
//  D3D12_HEAP_TYPE_DEFAULT,D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_MeshInfo.m_pd3dTextureCoord0UploadBuffer);
//	m_MeshInfo.m_d3dTextureCoord0BufferView.BufferLocation = m_MeshInfo.m_pd3dTextureCoord0Buffer->GetGPUVirtualAddress();
//	m_MeshInfo.m_d3dTextureCoord0BufferView.StrideInBytes = sizeof(XMFLOAT2);
//	m_MeshInfo.m_d3dTextureCoord0BufferView.SizeInBytes = sizeof(XMFLOAT2) * m_nVertices;
//}
//
//CBillboardRectMesh::~CBillboardRectMesh()
//{
//}
//
//void CBillboardRectMesh::ReleaseUploadBuffers()
//{
//	CMesh::ReleaseUploadBuffers();
//}
//
//void CBillboardRectMesh::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
//{
//	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
//
//	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[2] = { m_MeshInfo.m_d3dPositionBufferView, m_MeshInfo.m_d3dTextureCoord0BufferView };
//	pd3dCommandList->IASetVertexBuffers(m_nSlot, 2, pVertexBufferViews);
//
//	if ((m_MeshInfo.m_nSubMeshes > 0) && (nSubSet < m_MeshInfo.m_nSubMeshes))
//	{
//		pd3dCommandList->IASetIndexBuffer(&(m_MeshInfo.m_pd3dSubSetIndexBufferViews[nSubSet]));
//		pd3dCommandList->DrawIndexedInstanced(m_MeshInfo.m_pnSubSetIndices[nSubSet], 1, 0, 0, 0);
//	}
//	else
//	{
//		pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
//	}
//}

CTriangleRect::CTriangleRect(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, float fWidth, float fHeight, float fDepth)
{
	m_nVertices = 6;
	m_nStride = sizeof(CTexturedVertex);
	m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	CTexturedVertex pVertices[6];
	float fx = (fWidth * 0.5f);
	float fy = (fHeight * 0.5f);
	float fz = (fDepth * 0.5f);

	if (fDepth == 0.0f)
	{
		pVertices[0] = CTexturedVertex(XMFLOAT3(+fx , +fy , fz), XMFLOAT2(1.0f, 0.0f));
		pVertices[1] = CTexturedVertex(XMFLOAT3(+fx , -fy , fz), XMFLOAT2(1.0f, 1.0f));
		pVertices[2] = CTexturedVertex(XMFLOAT3(-fx , -fy , fz), XMFLOAT2(0.0f, 1.0f));
		pVertices[3] = CTexturedVertex(XMFLOAT3(-fx , -fy , fz), XMFLOAT2(0.0f, 1.0f));
		pVertices[4] = CTexturedVertex(XMFLOAT3(-fx , +fy , fz), XMFLOAT2(0.0f, 0.0f));
		pVertices[5] = CTexturedVertex(XMFLOAT3(+fx , +fy , fz), XMFLOAT2(1.0f, 0.0f));
	}

	m_pd3dVertexBuffer = CreateBufferResource(pd3dDevice, pd3dCommandList, pVertices, m_nStride * m_nVertices, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dVertexUploadBuffer);

	m_d3dVertexBufferView.BufferLocation = m_pd3dVertexBuffer->GetGPUVirtualAddress();
	m_d3dVertexBufferView.StrideInBytes = m_nStride;
	m_d3dVertexBufferView.SizeInBytes = m_nStride * m_nVertices;
}

CTriangleRect::~CTriangleRect()
{
}

void CTriangleRect::ReleaseUploadBuffers()
{
	if (m_pd3dVertexBuffer) m_pd3dVertexBuffer->Release();
	if (m_pd3dVertexUploadBuffer) m_pd3dVertexUploadBuffer->Release();
	CMesh::ReleaseUploadBuffers();
}

void CTriangleRect::Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet)
{
	pd3dCommandList->IASetPrimitiveTopology(m_d3dPrimitiveTopology);
	pd3dCommandList->IASetVertexBuffers(m_nSlot, 1, &m_d3dVertexBufferView);
	pd3dCommandList->DrawInstanced(m_nVertices, 1, m_nOffset, 0);
}
