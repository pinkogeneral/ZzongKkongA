#pragma once
#include "stdafx.h"
using namespace std;

class CPlayer;
class CGameScene;
class CCamera;
class CRotatingItem;

class CItemMgr
{

private:
	static CItemMgr* m_pInstance;
	CRotatingItem* m_pRotatingItem = NULL;

	int	m_iItemNum[3] = { 0,0,0 };

public:
	void Initialize(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignatureCPlayer);
	void BillboardUIRender(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void Update(float fElapsedTime);
	void GetItem(int iType, XMFLOAT3 Pos);
	void GiveItemToPlayer(int iType) { m_iItemNum[iType]++; }
	void UseItemToPlayer(int iType) { if(m_iItemNum[iType]>0) m_iItemNum[iType]--; }
	
	static CItemMgr* GetInstance(void)
	{
		if (m_pInstance == NULL)
			m_pInstance = new CItemMgr();
		return m_pInstance;
	}
	void Destroy();
	
};

