#include "pch.h"
#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"

CGameObject::CGameObject()
{
	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());
}

CGameObject::~CGameObject()
{
	//if(m_pMesh) m_pMesh->Release();
	if (m_pShader)
	{
		m_pShader->ReleaseShaderVariables();
		//m_pShader->Release();
	}
}

void CGameObject::SetShader(const std::shared_ptr<CShader>& pShader)
{
	//if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	//if (m_pShader) m_pShader->AddRef();
}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::SetMesh(const std::shared_ptr<CMesh>& pMesh)
{
	//if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	//if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject::SetMesh(std::shared_ptr<CMesh>&& pMesh)
{
	m_pMesh = std::move(pMesh);
}

void CGameObject::ReleaseUploadBuffers()
{
	//정점 버퍼를 위한 업로드 버퍼를 소멸시킨다.
	if (m_pMesh.get()) m_pMesh->ReleaseUploadBuffers();
}

void CGameObject::Animate(float fTimeElapsed)
{
}

void CGameObject::OnPrepareRender()
{
}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pShader.get()) 
	{
		m_pShader->UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);
		m_pShader->Render(pd3dCommandList, pCamera);
	}

	if (m_pMesh.get()) m_pMesh->Render(pd3dCommandList);
}


//================================================================================================
//================================================================================================
//================================================================================================
CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float FTimeElapsed)
{
	CGameObject::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * FTimeElapsed);
}