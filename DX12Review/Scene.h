#pragma once

#include "Timer.h"
#include "Shader.h"

class CCamera;
class CGameObject;

struct LIGHT
{
	// 16바이트 정렬
	XMFLOAT4	m_xmf4Ambient;

	XMFLOAT4	m_xmf4Diffuse;

	XMFLOAT4	m_xmf4Specular;

	XMFLOAT3	m_xmf3Position;
	float		m_fFalloff;

	XMFLOAT3	m_xmf3Direction;
	float		m_fTheta;	//cos(m_fTheta)

	XMFLOAT3	m_xmf3Attenuation;
	float		m_fPhi;	//cos(m_fPhi)

	bool		m_bEnable;
	int			m_nType;
	float		m_fRange;
	float		padding;
};

struct LIGHTS
{
	LIGHT		m_pLights[MAX_LIGHTS];
	XMFLOAT4	m_xmf4GlobalAmbient;
};

struct MATERIALS
{
	MATERIAL	m_pReflections[MAX_MATERIALS];
};

class CScene
{
public:
	CScene();
	~CScene();

	//씬에서 마우스와 키보드 메시지를 처리한다
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void ReleaseUploadBuffers();

	//그래픽 루트 시그니쳐를 생성한다.
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsrootSignature();

	//씬의 모든 조명과 재질을 생성
	void BuildLightsAndMaterials();

	//씬의 모든 조명과 재질을 위한 리소스를 생성하고 갱신
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandlist);
	virtual void ReleaseShaderVariables();

	std::shared_ptr<CPlayer>	m_pPlayer;

protected:
	//씬은 게임 객체들의 집합이다. 게임 객체는 셰이더를 포함한다.
	std::vector<CObjectsShader> m_vShaders;

	ComPtr<ID3D12RootSignature> m_pd3dGraphicsRootSignature;

	//씬의 조명
	std::shared_ptr<LIGHTS>		m_pLights;

	//조명을 나타내는 리소스와 리소스에 대한 포인터이다.
	ComPtr<ID3D12Resource>		m_pd3dcbLights;
	LIGHTS*						m_pcbMappedLights;

	//씬의 객체들에 적용되는 재질
	std::shared_ptr<MATERIALS>	m_pMaterials;

	//재질을 나타내는 리소스와 리소스에 대한 포인터이다.
	ComPtr<ID3D12Resource>		m_pd3dcbMaterials;
	MATERIALS*					m_pcbMappedMaterials;

	ComPtr<ID3D12DescriptorHeap>	m_pd3dCbvSrvDescriptorHeap;

	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dObjectsCbvCPUDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dObjectsCbvGPUDescriptorHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dLightsCbvCPUDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dLightsCbvGPUDescriptorHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE		m_d3dMaterialsCbvCPUDescriptorHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dMaterialsCbvGPUDescriptorHandle;
};