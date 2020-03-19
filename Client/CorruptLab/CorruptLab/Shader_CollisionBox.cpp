#include "stdafx.h"
#include "Shader_CollisionBox.h"

Shader_CollisionBox::Shader_CollisionBox()
{
}

Shader_CollisionBox::~Shader_CollisionBox()
{
}


D3D12_INPUT_LAYOUT_DESC Shader_CollisionBox::CreateInputLayout()
{
	UINT nInputElementDescs = 3;
	D3D12_INPUT_ELEMENT_DESC* pd3dInputElementDescs = new D3D12_INPUT_ELEMENT_DESC[nInputElementDescs];
	pd3dInputElementDescs[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[1] = { "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };
	pd3dInputElementDescs[2] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 };

	D3D12_INPUT_LAYOUT_DESC d3dInputLayoutDesc;
	d3dInputLayoutDesc.pInputElementDescs = pd3dInputElementDescs;
	d3dInputLayoutDesc.NumElements = nInputElementDescs;
	return(d3dInputLayoutDesc);
}
D3D12_SHADER_BYTECODE Shader_CollisionBox::CreateVertexShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"HLSL_Collision.hlsl", "VSCollisionBox", "vs_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Shader_CollisionBox::CreatePixelShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"HLSL_Collision.hlsl", "PSCollisionBox", "ps_5_1", ppd3dShaderBlob));
}

D3D12_SHADER_BYTECODE Shader_CollisionBox::CreateGeometryShader(ID3DBlob** ppd3dShaderBlob)
{
	return(CShader::CompileShaderFromFile(L"HLSL_Collision.hlsl", "GS", "gs_5_1", ppd3dShaderBlob));
}

void Shader_CollisionBox::CreateShader(ID3D12Device* pd3dDevice, ID3D12RootSignature* pd3dGraphicsRootSignature, UINT nRenderTargets)
{
	m_nPipelineStates = 1;
	m_ppd3dPipelineStates = new ID3D12PipelineState * [m_nPipelineStates];

	ID3DBlob* pd3dVertexShaderBlob = NULL; 
	ID3DBlob* pd3dPixelShaderBlob = NULL;
    ID3DBlob* pd3dGeometryShaderBlob = NULL;

	::ZeroMemory(&m_d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	m_d3dPipelineStateDesc.pRootSignature = pd3dGraphicsRootSignature;
	m_d3dPipelineStateDesc.VS = CreateVertexShader(&pd3dVertexShaderBlob);
	m_d3dPipelineStateDesc.PS = CreatePixelShader(&pd3dPixelShaderBlob);
	m_d3dPipelineStateDesc.GS = CreateGeometryShader(&pd3dGeometryShaderBlob);
	m_d3dPipelineStateDesc.RasterizerState = CreateRasterizerState();
	m_d3dPipelineStateDesc.BlendState = CreateBlendState();
	m_d3dPipelineStateDesc.DepthStencilState = CreateDepthStencilState();
	m_d3dPipelineStateDesc.InputLayout = CreateInputLayout();
	m_d3dPipelineStateDesc.SampleMask = UINT_MAX;
	m_d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	m_d3dPipelineStateDesc.NumRenderTargets = 1;
	m_d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	m_d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	m_d3dPipelineStateDesc.SampleDesc.Count = 1;
	m_d3dPipelineStateDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;

	HRESULT hResult = pd3dDevice->CreateGraphicsPipelineState(&m_d3dPipelineStateDesc,
		__uuidof(ID3D12PipelineState), (void**)&m_ppd3dPipelineStates[0]);

	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();
	if (pd3dGeometryShaderBlob) pd3dGeometryShaderBlob->Release();
	if (m_d3dPipelineStateDesc.InputLayout.pInputElementDescs)
		delete[] m_d3dPipelineStateDesc.InputLayout.pInputElementDescs;
}

void Shader_CollisionBox::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, void* pContext)
{
	for (int i = 0; i < m_nInstance; ++i)
	{
		std::cout <<"Center : "<< m_pBoxInfo[i].m_xmf3Center.x << "	"<< m_pBoxInfo[i].m_xmf3Center.y <<"	"<< m_pBoxInfo[i].m_xmf3Center.z <<std::endl;
		std::cout << "Extent : " << m_pBoxInfo[i].m_xmf3Extent.x << "	" << m_pBoxInfo[i].m_xmf3Extent.y << "	" << m_pBoxInfo[i].m_xmf3Extent.z << std::endl<< std::endl;

	}
	m_pd3dCollisionBuffer = ::CreateBufferResource(pd3dDevice, pd3dCommandList, m_pBoxInfo,
		sizeof(GS_COLLISION_BOX_INFO) * m_nInstance, D3D12_HEAP_TYPE_DEFAULT, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, &m_pd3dCollisionUploadBuffer);

	m_d3dCollisionBufferView.BufferLocation = m_pd3dCollisionBuffer->GetGPUVirtualAddress();
	m_d3dCollisionBufferView.StrideInBytes = sizeof(GS_COLLISION_BOX_INFO);
	m_d3dCollisionBufferView.SizeInBytes = sizeof(GS_COLLISION_BOX_INFO) * m_nInstance;
}

void Shader_CollisionBox::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CShader::Render(pd3dCommandList, pCamera);

	D3D12_VERTEX_BUFFER_VIEW pVertexBufferViews[1] = { m_d3dCollisionBufferView };
	pd3dCommandList->IASetVertexBuffers(0, _countof(pVertexBufferViews), pVertexBufferViews);
	pd3dCommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_POINTLIST);
	pd3dCommandList->DrawInstanced(1, m_nInstance, 0, 0);
}