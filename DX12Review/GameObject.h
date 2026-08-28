#pragma once

#include "Mesh.h"

class CShader;
class CCamera;

struct MATERIAL
{
	XMFLOAT4	m_xmf4Ambient;
	XMFLOAT4	m_xmf4Diffuse;
	XMFLOAT4	m_xmf4Specular; //(r, g, b, a = power)
	XMFLOAT4	m_xmf4Emissive;
};

class CMaterial
{
public:
	CMaterial();
	virtual ~CMaterial();

	//재질의 기본 색상
	XMFLOAT4					m_xmf4Albedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

	//재질의 번호
	UINT						m_nReflection = 0;
	//재질을 적용해 렌더링 하기위한 쉐이더
	std::shared_ptr<CShader>	m_pShader;

	void SetAlbedo(XMFLOAT4& xmf4Albedo) { m_xmf4Albedo = xmf4Albedo; }
	void SetReflection(UINT nReflection) { m_nReflection = nReflection; }
	void SetShader(const std::shared_ptr<CShader>& pShader);
};

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	void ReleaseUploadBuffers();

	virtual void SetMesh(const std::shared_ptr<CMesh>& pMesh);
	virtual void SetMesh(std::shared_ptr<CMesh>&& pMesh);
	virtual void SetShader(const std::shared_ptr<CShader>& pShader);
	void SetMaterial(std::shared_ptr<CMaterial>& pMaterial);
	void SetMaterial(UINT nReflection);

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	//상수 버퍼를 생성한다.
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//상수 버퍼의 내용을 갱신한다.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	//게임 객체의 월드 변환 행렬에서 위치 벡터와 방향 벡터를 반환한다.
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();
	const XMFLOAT4X4& GetWorldMatrix() { return m_xmf4x4World; }
	std::shared_ptr<CMaterial>& GetMaterial() { return m_pMaterial; }

	//게임 객체의 위치를 설정한다.
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);

	//게임 객체를 로컬 x, y, z축 방향으로 이동한다.
	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);

	//게임 객체를 회전(x, y, z)한다
	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);

	D3D12_GPU_DESCRIPTOR_HANDLE	m_d3dCbvGPUDescriptorHandle;
	void SetCbvGPUDescriptorHandle(UINT64 nCbvGPUDescriptorHandlePtr) {	m_d3dCbvGPUDescriptorHandle.ptr = nCbvGPUDescriptorHandlePtr; }
	D3D12_GPU_DESCRIPTOR_HANDLE GetCbvGPUDescriptorHandle() { return m_d3dCbvGPUDescriptorHandle; }

protected:
	XMFLOAT4X4 m_xmf4x4World;

	std::shared_ptr<CMesh> m_pMesh;

	//std::shared_ptr<CShader> m_pShader;
	std::shared_ptr<CMaterial> m_pMaterial;
};

class CRotatingObject : public CGameObject
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

	void SetRotationSpeed(float FRotationSpeed) { m_fRotationSpeed = FRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed);

private:
	XMFLOAT3 m_xmf3RotationAxis;
	float m_fRotationSpeed = 0;
};