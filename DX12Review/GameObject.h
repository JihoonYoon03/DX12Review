#pragma once

#include "Mesh.h"

class CShader;
class CCamera;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	void ReleaseUploadBuffers();

	virtual void SetMesh(const std::shared_ptr<CMesh>& pMesh);
	virtual void SetMesh(std::shared_ptr<CMesh>&& pMesh);
	virtual void SetShader(const std::shared_ptr<CShader>& pShader);

	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

protected:
	XMFLOAT4X4 m_xmf4x4World;

	std::shared_ptr<CMesh> m_pMesh = NULL;

	std::shared_ptr<CShader> m_pShader = NULL;
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
	float m_fRotationSpeed;
};