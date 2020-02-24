//------------------------------------------------------- ----------------------
// File: Mesh.h
//-----------------------------------------------------------------------------
#pragma once

class CGameObject; 

// m_ntype�� ����
#define VERTEXT_POSITION				0x0001
#define VERTEXT_COLOR					0x0002
#define VERTEXT_NORMAL					0x0004
#define VERTEXT_TANGENT                 0x0008 
#define VERTEXT_TEXTURE_COORD0			0x0010
#define VERTEXT_TEXTURE_COORD1			0x0020

#define VERTEXT_BONE_INDEX_WEIGHT		0x1000

#define VERTEXT_TEXTURE					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_DETAIL					(VERTEXT_POSITION | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TEXTURE			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_TANGENT_TEXTURE	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0)
#define VERTEXT_NORMAL_DETAIL			(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)
#define VERTEXT_NORMAL_TANGENT__DETAIL	(VERTEXT_POSITION | VERTEXT_NORMAL | VERTEXT_TANGENT | VERTEXT_TEXTURE_COORD0 | VERTEXT_TEXTURE_COORD1)


typedef struct MESHINFO
{

	XMFLOAT3                      * m_pxmf3Positions = NULL;
	ID3D12Resource                * m_pd3dVertexBuffer = NULL;
	ID3D12Resource                * m_pd3dVertexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW        m_d3dVertexBufferView;
				                    
	ID3D12Resource                * m_pd3dIndexBuffer = NULL;
	ID3D12Resource                * m_pd3dIndexUploadBuffer = NULL;
	D3D12_INDEX_BUFFER_VIEW	        m_d3dIndexBufferView;
							      
	XMFLOAT4                      * m_pxmf4Colors = NULL;
	ID3D12Resource                * m_pd3dColorBuffer = NULL;
	ID3D12Resource                * m_pd3dColorUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dColorBufferView;

	XMFLOAT2                      * m_pxmf2TextureCoords0 = NULL;
	ID3D12Resource                * m_pd3dTextureCoord0Buffer = NULL;
	ID3D12Resource                * m_pd3dTextureCoord0UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord0BufferView;

	XMFLOAT2                      * m_pxmf2TextureCoords1 = NULL;
	ID3D12Resource                * m_pd3dTextureCoord1Buffer = NULL;
	ID3D12Resource                * m_pd3dTextureCoord1UploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTextureCoord1BufferView;

	XMFLOAT3                      * m_pxmf3Normals = NULL;
	ID3D12Resource                * m_pd3dNormalBuffer = NULL;
	ID3D12Resource                * m_pd3dNormalUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dNormalBufferView;

	XMFLOAT3                      * m_pxmf3Tangents = NULL;
	ID3D12Resource                * m_pd3dTangentBuffer = NULL;
	ID3D12Resource                * m_pd3dTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dTangentBufferView;

	XMFLOAT3                      * m_pxmf3BiTangents = NULL;
	ID3D12Resource                * m_pd3dBiTangentBuffer = NULL;
	ID3D12Resource                * m_pd3dBiTangentUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBiTangentBufferView;

	int                             m_nSubMeshes = 0;
	int                           * m_pnSubSetIndices = NULL;
	UINT                         ** m_ppnSubSetIndices = NULL;
	ID3D12Resource               ** m_ppd3dSubSetIndexBuffers = NULL;
	ID3D12Resource               ** m_ppd3dSubSetIndexUploadBuffers = NULL;
	D3D12_INDEX_BUFFER_VIEW       * m_pd3dSubSetIndexBufferViews = NULL;
	
	char                            m_pstrMeshName[256] = { 0 };

	XMFLOAT3             	        m_xmf3AABBCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);
	XMFLOAT3             	        m_xmf3AABBExtents = XMFLOAT3(0.0f, 0.0f, 0.0f);

	D3D12_PRIMITIVE_TOPOLOGY        m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
							      
	UINT					        m_nSlot = 0;
	UINT					        m_nVertices = 0;
	UINT					        m_nStride = 0;
	UINT					        m_nOffset = 0;
							      
	UINT					        m_nIndices = 0;
	UINT                          * m_pnIndices = NULL;
	UINT					        m_nStartIndex = 0;
	UINT                            m_nType = 0;
	int						        m_nBaseVertex = 0;
} MeshInfo;


class CStandardVertex
{
public:
	XMFLOAT3				        m_xmf3Position;
	XMFLOAT3				        m_xmf3Normal;
	XMFLOAT2				        m_xmf2TexCoord;

public:
	CStandardVertex()
	{
		m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf3Normal = XMFLOAT3(0.0f, 0.0f, 0.0f);
		m_xmf2TexCoord = XMFLOAT2(0.0f, 0.0f);
	}
	~CStandardVertex() { }
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMesh
{
public:
	CMesh(ID3D12Device *pd3dDevice, ID3D12GraphicsCommandList *pd3dCommandList);
	virtual ~CMesh();

private:
	int	m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void ReleaseUploadBuffers();
	MeshInfo* GetMeshInfo() { return &m_MeshInfo; }

protected:
	MeshInfo              m_MeshInfo;
public:
	UINT GetType() { return(m_MeshInfo.m_nType); }
	virtual void Render(ID3D12GraphicsCommandList *pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet) { }
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CStandardMesh : public CMesh
{
public:
	CStandardMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CStandardMesh();

	virtual void ReleaseUploadBuffers();

public:
	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};

#define SKINNED_ANIMATION_BONES		128

class CSkinnedMesh : public CStandardMesh
{
public:
	CSkinnedMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual ~CSkinnedMesh();

protected:
	char							m_pstrSkinnedMeshName[64] = { 0 };

	int								m_nBonesPerVertex = 4;

	XMUINT4* m_pxmu4BoneIndices = NULL;
	XMFLOAT4* m_pxmf4BoneWeights = NULL;

	ID3D12Resource* m_pd3dBoneIndexBuffer = NULL;
	ID3D12Resource* m_pd3dBoneIndexUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneIndexBufferView;

	ID3D12Resource* m_pd3dBoneWeightBuffer = NULL;
	ID3D12Resource* m_pd3dBoneWeightUploadBuffer = NULL;
	D3D12_VERTEX_BUFFER_VIEW		m_d3dBoneWeightBufferView;

public:
	int								m_nSkinningBones = 0;

	char(*m_ppstrSkinningBoneNames)[64];
	XMFLOAT4X4* m_pxmf4x4BindPoseBoneOffsets = NULL;

	CGameObject** m_ppSkinningBoneFrameCaches = NULL;

	ID3D12Resource* m_pd3dcbBoneOffsets = NULL;
	XMFLOAT4X4* m_pcbxmf4x4BoneOffsets = NULL;

	ID3D12Resource* m_pd3dcbBoneTransforms = NULL;
	XMFLOAT4X4* m_pcbxmf4x4BoneTransforms = NULL;
public:
	void PrepareSkinning(CGameObject* pModelRootObject);

	void LoadSkinInfoFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, FILE* pInFile);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	virtual void ReleaseUploadBuffers();

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);
};