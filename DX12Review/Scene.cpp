#include "pch.h"
#include "Scene.h"
#include "Mesh.h"
#include "Shader.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return false;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature;

	//매개변수가 없는 루트 시그니쳐를 생성한다
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = 0;
	d3dRootSignatureDesc.pParameters = NULL;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	HRESULT hr = ::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	if (FAILED(hr))
	{
		OutputDebugString(L"SerializeRootSignature Failed\n");
	}
	hr = pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), IID_PPV_ARGS(&pd3dGraphicsRootSignature));
	if (FAILED(hr))
	{
		OutputDebugString(L"CreateRootSignature Failed\n");
	}
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return pd3dGraphicsRootSignature;
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	std::shared_ptr<CShader> pShader = std::make_shared<CShader>();
	pShader->CreateShader(pd3dDevice, m_pd3dGraphicsRootSignature.Get());
	pShader->BuildObjects(pd3dDevice, pd3dCommandList, NULL);

	m_vpShaders.push_back(std::move(pShader));
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature.Reset();

	for (std::shared_ptr<CShader>& shader : m_vpShaders)
	{
		if (shader.get()) 
		{
			shader->ReleaseShaderVariables();
			shader->ReleaseObjects();
		}
	}
	m_vpShaders.clear();
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return false;
}

void CScene::AnimateObjects(float fTimeElapsed)
{
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList)
{
	//그래픽 루트 시그니쳐를 파이프라인에 연결(설정)한다.
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature.Get());

	for (std::shared_ptr<CShader>& shader : m_vpShaders)
	{
		shader->Render(pd3dCommandList);
	}
}

void CScene::ReleaseUploadBuffers()
{
	for (std::shared_ptr<CShader>& shader : m_vpShaders)
	{
		if (shader.get()) shader->ReleaseUploadBuffers();
	}
}

ID3D12RootSignature* CScene::GetGraphicsrootSignature()
{
	return m_pd3dGraphicsRootSignature.Get();
}