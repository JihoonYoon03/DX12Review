#pragma once

#include "Mesh.h"

class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

	void ReleaseUploadBuffers();

	virtual void SetMesh(const std::shared_ptr<CMesh>& pMesh);
	virtual void SetMesh(std::shared_ptr<CMesh>&& pMesh);
	virtual void SetShader(const std::shared_ptr<CShader>& pShader);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	XMFLOAT4X4 m_xmf4x4World;

	std::shared_ptr<CMesh> m_pMesh = NULL;

	std::shared_ptr<CShader> m_pShader = NULL;
};

