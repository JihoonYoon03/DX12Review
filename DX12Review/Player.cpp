#include "pch.h"
#include "Player.h"
#include "Shader.h"

CPlayer::CPlayer(int nMeshes)
	:CGameObject(nMeshes)
{
	m_xmf3Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Right = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_xmf3Up = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_xmf3Look = XMFLOAT3(0.0f, 0.0f, 1.0f);

	m_xmf3Velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_xmf3Gravity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_fMaxVelocityXZ = 0.0f;
	m_fMaxVelocityY = 0.0f;
	m_fFriction = 0.0f;

	m_fPitch = 0.0f;
	m_fRoll = 0.0f;
	m_fYaw = 0.0f;

	m_pPlayerUpdatedContext = nullptr;
	m_pCameraUpdatedContext = nullptr;
}

CPlayer::~CPlayer()
{
	ReleaseShaderVariables();

	if (m_pCamera) m_pCamera.reset();
}

//플레이어 위치를 변경하는 함수. 이동방향에 따라 fDistance만큼 이동
void CPlayer::Move(DWORD dwDirection, float fDistance, bool bUpdateVelocity)
{
	if (dwDirection)
	{
		XMFLOAT3 xmf3Shift = XMFLOAT3(0.0f, 0.0f, 0.0f);
		//↑ = +z, ↓ = -z
		if (dwDirection & DIR_FORWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, fDistance);
		if (dwDirection & DIR_BACKWARD) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Look, -fDistance);
		//→ = +x, ← = -x
		if (dwDirection & DIR_RIGHT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, fDistance);
		if (dwDirection & DIR_LEFT) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Right, -fDistance);
		//Page Up = +y, Page Down = -y
		if (dwDirection & DIR_UP) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, fDistance);
		if (dwDirection & DIR_DOWN) xmf3Shift = Vector3::Add(xmf3Shift, m_xmf3Up, -fDistance);

		Move(xmf3Shift, bUpdateVelocity);
	}
}

void CPlayer::Move(const XMFLOAT3& xmf3Shift, bool bVelocity)
{
	//bVelocity가 참이면 플레이어를 이동하지 않고 속도 벡터를 변경
	if (bVelocity)
	{
		//플레이어 속도 벡터를 xmf3Shift 벡터만큼 변경
		m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, xmf3Shift);
	}
	else
	{
		//플레이어를 현재 위치 벡터에서 xmf3Shift 벡터만큼 이동
		m_xmf3Position = Vector3::Add(m_xmf3Position, xmf3Shift);
		if (m_pCamera) m_pCamera->Move(xmf3Shift);
	}
}

void CPlayer::Move(float fxOffset, float fyOffset, float fzOffset)
{
}

void CPlayer::Rotate(float x, float y, float z)
{
	DWORD nCameraMode = m_pCamera->GetMode();
	//1인칭 or 3인칭 카메라인 경우 플레이어 회전에 제약이 있다.
	if (nCameraMode == FIRST_PERSON_CAMERA || nCameraMode == THIRD_PERSON_CAMERA)
	{
		//x축 중심 회전 = 고개를 앞뒤로 숙이는 것이므로 -89~89도로 각도 제한
		if (x != 0.0f)
		{
			m_fPitch += x;
			if (m_fPitch > 89.0f)
			{
				x -= (m_fPitch - 89.0f);
				m_fPitch = 89.0f;
			}
			if (m_fPitch < -89.0f)
			{
				x -= (m_fPitch + 89.0f);
				m_fPitch = -89.0f;
			}
		}
		if (y != 0.0f)
		{
			m_fYaw += y;
			if (m_fYaw > 360.0f) m_fYaw -= 360.0f;
			if (m_fYaw < 0.0f) m_fYaw += 360.0f;
		}
		if (z != 0.0f)
		{
			//z축 중심 회전 = 몸통을 좌우로 기울이는 것이므로 -20~20도로 각도 제한
			m_fRoll += z;
			if (m_fRoll > 20.0f)
			{
				z -= (m_fRoll - 20.0f);
				m_fRoll = 20.0f;
			}
			if (m_fRoll < -20.0f)
			{
				z -= (m_fRoll + 20.0f);
				m_fRoll = -20.0f;
			}
		}

		//카메라를 x, y, z만큼 회전한다
		m_pCamera->Rotate(x, y, z);

		//플레이어를 회전한다. 1인칭/3인치에서 플레이어 자체의 회전은 y축에 대해서만 일어난다.
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}
	else if (nCameraMode == SPACESHIP_CAMERA)
	{
		//우주선 카메라는 자유 회전이 가능하다.
		m_pCamera->Rotate(x, y, z);
		if (x != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Right), XMConvertToRadians(x));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
		}
		if (y != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Up), XMConvertToRadians(y));
			m_xmf3Look = Vector3::TransformNormal(m_xmf3Look, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
		if (z != 0.0f)
		{
			XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3Look), XMConvertToRadians(z));
			m_xmf3Up = Vector3::TransformNormal(m_xmf3Up, xmmtxRotate);
			m_xmf3Right = Vector3::TransformNormal(m_xmf3Right, xmmtxRotate);
		}
	}

	//회전 이후 좌표축 직교하도록 보정
	m_xmf3Look = Vector3::Normalize(m_xmf3Look);
	m_xmf3Right = Vector3::CrossProduct(m_xmf3Up, m_xmf3Look, true);
	m_xmf3Up = Vector3::CrossProduct(m_xmf3Look, m_xmf3Right, true);
}

//매 프레임마다 호출되는 함수이다.
void CPlayer::Update(float fTimeElapsed)
{
	//플레이어 속도 벡터를 중력 벡터와 더함.
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Gravity, fTimeElapsed, false));

	//플레이어 속도 벡터의 xz 성분 크기를 구하고, 최대 속력보다 클 경우 조정
	float fLength = sqrtf(m_xmf3Velocity.x * m_xmf3Velocity.x + m_xmf3Velocity.z * m_xmf3Velocity.z);
	float fMaxVelocityXZ = m_fMaxVelocityXZ * fTimeElapsed;
	if (fLength > m_fMaxVelocityXZ)
	{
		m_xmf3Velocity.x *= (fMaxVelocityXZ / fLength);
		m_xmf3Velocity.z *= (fMaxVelocityXZ / fLength);
	}
	//플레이어 속도 벡터의 y 성분 크기를 구하고, 최대 속력보다 클 경우 조정
	float fMaxVelocityY = m_fMaxVelocityY * fTimeElapsed;
	fLength = sqrtf(m_xmf3Velocity.y * m_xmf3Velocity.y);
	if (fLength > m_fMaxVelocityY) m_xmf3Velocity.y *= (fMaxVelocityY / fLength);

	//플레이어를 속도 벡터 만큼 실제로 이동한다
	XMFLOAT3 xmf3Velocity = Vector3::ScalarProduct(m_xmf3Velocity, fTimeElapsed, false);
	Move(xmf3Velocity, false);

	//플레이어 위치가 변경될 때 추가로 수행할 작업
	//유효한 위치 여부 검사, 충돌 검사
	if (m_pPlayerUpdatedContext) OnPlayerUpdateCallback(fTimeElapsed);

	DWORD nCameraMode = m_pCamera->GetMode();
	//3인칭 카메라 위치 갱신
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->Update(m_xmf3Position, fTimeElapsed);

	//카메라 위치가 변경될 때 추가로 수행할 작업
	if (m_pCameraUpdatedContext) OnCameraUpdateCallback(fTimeElapsed);
	//3인칭 카메라 방향 조정
	if (nCameraMode == THIRD_PERSON_CAMERA) m_pCamera->SetLookAt(m_xmf3Position);
	//카메라 변환 행렬 재생성(좌표축 직교하도록 조정)
	m_pCamera->RegenerateViewMatrix();

	//플레이어 속도 벡터가 마찰을 받아 감속되어야 한다면, 감속 벡터 생성. 마찰 > 속력 시 속력은 0이 된다
	fLength = Vector3::Length(m_xmf3Velocity);
	float fDeceleration = m_fFriction * fTimeElapsed;
	if (fDeceleration > fLength) fDeceleration = fLength;
	m_xmf3Velocity = Vector3::Add(m_xmf3Velocity, Vector3::ScalarProduct(m_xmf3Velocity, -fDeceleration, true));
}

void CPlayer::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::CreateShaderVariables(pd3dDevice, pd3dCommandList);

	if (m_pCamera) m_pCamera->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CPlayer::ReleaseShaderVariables()
{
	CGameObject::ReleaseShaderVariables();

	if (m_pCamera) m_pCamera->ReleaseShaderVariables();
}

void CPlayer::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	CGameObject::UpdateShaderVariables(pd3dCommandList);
}

std::shared_ptr<CCamera> CPlayer::OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode)
{
	//모드에 맞는 카메라 생성
	std::shared_ptr<CCamera> pNewCamera;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		pNewCamera = std::make_shared<CFirstPersonCamera>(m_pCamera.get());
		break;
	case THIRD_PERSON_CAMERA:
		pNewCamera = std::make_shared<CThirdPersonCamera>(m_pCamera.get());
		break;
	case SPACESHIP_CAMERA:
		pNewCamera = std::make_shared<CSpaceShipCamera>(m_pCamera.get());
		break;
	}

	//현재 카메라 모드가 우주선, 새로운 카메라가 1인칭 or 3인칭이면 플레이어 Up 벡터는 월드좌표계 y축 방향 벡터가 되도록 함.
	if (nCurrentCameraMode == SPACESHIP_CAMERA)
	{
		m_xmf3Right = Vector3::Normalize(XMFLOAT3(m_xmf3Right.x, 0.0f, m_xmf3Right.z));
		m_xmf3Up = Vector3::Normalize(XMFLOAT3(0.0f, 1.0f, 0.0f));
		m_xmf3Look = Vector3::Normalize(XMFLOAT3(m_xmf3Look.x, 0.0f, m_xmf3Look.z));
		
		m_fPitch = 0.0f;
		m_fRoll = 0.0f;

		//y축 회전각 설정
		m_fYaw = Vector3::Angle(XMFLOAT3(0.0f, 0.0f, 1.0f), m_xmf3Look);
		//좌우 판단
		if (m_xmf3Look.x < 0.0f) m_fYaw = -m_fYaw;
	}
	//새로운 카메라 모드가 우주선 카메라이고 현재가 1인칭 or 3인칭이면 플레이어 로컬 축 = 카메라 로컬 축으로 만듦
	else if ((nNewCameraMode == SPACESHIP_CAMERA) && m_pCamera)
	{
		m_xmf3Right = m_pCamera->GetRightVector();
		m_xmf3Up = m_pCamera->GetUpVector();
		m_xmf3Look = m_pCamera->GetLookVector();
	}

	if (pNewCamera)
	{
		pNewCamera->SetMode(nNewCameraMode);
		pNewCamera->SetPlayer(this);
	}
	
	if (m_pCamera) m_pCamera.reset();

	return pNewCamera;
}

//플레이어 위치와 회전축으로부터 월드 변환 행렬을 생성
void CPlayer::OnPrepareRender()
{
	m_xmf4x4World._11 = m_xmf3Right.x;
	m_xmf4x4World._12 = m_xmf3Right.y;
	m_xmf4x4World._13 = m_xmf3Right.z;
	m_xmf4x4World._21 = m_xmf3Up.x;
	m_xmf4x4World._22 = m_xmf3Up.y;
	m_xmf4x4World._23 = m_xmf3Up.z;
	m_xmf4x4World._31 = m_xmf3Look.x;
	m_xmf4x4World._32 = m_xmf3Look.y;
	m_xmf4x4World._33 = m_xmf3Look.z;
	m_xmf4x4World._41 = m_xmf3Position.x;
	m_xmf4x4World._42 = m_xmf3Position.y;
	m_xmf4x4World._43 = m_xmf3Position.z;
}

void CPlayer::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	DWORD nCameraMode = (pCamera) ? pCamera->GetMode() : 0x00;
	//3인칭이면 플레이어 객체 렌더링
	if (nCameraMode == THIRD_PERSON_CAMERA)
	{
		if (m_pShader) m_pShader->Render(pd3dCommandList, pCamera);
		CGameObject::Render(pd3dCommandList, pCamera);
	}
}



//================================================================================================
//================================================================================================
//================================================================================================
CAirplanePlayer::CAirplanePlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, int nMeshes)
	: CPlayer(nMeshes)
{
	//비행기 메쉬를 생성
	std::shared_ptr<CMesh> pAirplaneMesh = std::make_shared<CAirplaneMeshDiffused>(pd3dDevice, pd3dCommandList, 20.0f, 20.0f, 4.0f, XMFLOAT4(0.0f, 0.5f, 0.0f, 0.0f));

	SetMesh(0, pAirplaneMesh);
	//플레이어의 카메라를 우주선 카메라로 변경한다
	m_pCamera = ChangeCamera(SPACESHIP_CAMERA, 0.0f);
	
	//플레이어를 위한 셰이더 변수를 생성한다.
	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//플레이어의 위치를 설정한다
	SetPosition(XMFLOAT3(0.0f, 0.0f, -50.0f));

	//플레이어 메쉬를 렌더링할 때 사용할 셰이더를 생성한다.
	std::shared_ptr<CPlayerShader> pShader = std::make_shared<CPlayerShader>();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);
}

CAirplanePlayer::~CAirplanePlayer()
{
}

void CAirplanePlayer::OnPrepareRender()
{
	CPlayer::OnPrepareRender();

	//비행기 모델을 그리기 전 x축으로 90도 회전한다.
	//비행기 모델 메쉬의 y축 방향이 비행기 앞쪽이 되도록 모델링 되었기 때문임.
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(90.0f), 0.0f, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

//카메라 변경 시 호출되는 함수이다
std::shared_ptr<CCamera> CAirplanePlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return m_pCamera;
	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(200.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT, 0.0f, 1.0f);
		m_pCamera->SetScissorRect(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT);
		break;

	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(400.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT, 0.0f,	1.0f);
		m_pCamera->SetScissorRect(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT);
		break;

	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(125.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		//3인칭 카메라의 지연 효과를 설정한다.
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 5000.0f, ASPECT_RATIO, 60.0f);
		m_pCamera->SetViewport(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT, 0.0f,	1.0f);
		m_pCamera->SetScissorRect(0, 0, Config::FRAME_BUFFER_WIDTH, Config::FRAME_BUFFER_HEIGHT);
		break;
	default:
		break;
	}
	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return m_pCamera;
}



//================================================================================================
//================================================================================================
//================================================================================================
CTerrainPlayer::CTerrainPlayer(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, void* pContext, int nMeshes)
	: CPlayer(nMeshes)
{
	m_pCamera = ChangeCamera(THIRD_PERSON_CAMERA, 0.0f);
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)pContext;

	//플레이어의 위치를 지형의 가운데(y-축 좌표는 지형의 높이보다 1500 높게)로 설정한다. 플레이어 위치 벡터의 y좌표가 지형의 높이보다 크고 중력이 작용하도록 플레이어를 설정하였으므로 플레이어는 점차적으로 하강하게 된다.*/
	float fHeight = pTerrain->GetHeight(pTerrain->GetWidth() * 0.5f, pTerrain->GetLength() * 0.5f);
	SetPosition(XMFLOAT3(pTerrain->GetWidth() * 0.5f, fHeight + 1500.0f, pTerrain->GetLength() * 0.5f));
	
	//플레이어의 위치가 변경될 때 지형의 정보에 따라 플레이어의 위치를 변경할 수 있도록 설정한다.
	SetPlayerUpdatedContext(pTerrain);
	//카메라의 위치가 변경될 때 지형의 정보에 따라 카메라의 위치를 변경할 수 있도록 설정한다. 
	SetCameraUpdatedContext(pTerrain);

	std::shared_ptr<CCubeMeshDiffused> pCubeMesh = std::make_shared<CCubeMeshDiffused>(pd3dDevice, pd3dCommandList, 4.0f, 12.0f, 4.0f);
	SetMesh(0, pCubeMesh);

	//플레이어를 렌더링할 셰이더를 생성한다.
	std::shared_ptr<CPlayerShader> pShader = std::make_shared<CPlayerShader>();
	pShader->CreateShader(pd3dDevice, pd3dGraphicsRootSignature);
	SetShader(pShader);

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

CTerrainPlayer::~CTerrainPlayer()
{
}

std::shared_ptr<CCamera> CTerrainPlayer::ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed)
{
	DWORD nCurrentCameraMode = (m_pCamera) ? m_pCamera->GetMode() : 0x00;
	if (nCurrentCameraMode == nNewCameraMode) return m_pCamera;

	switch (nNewCameraMode)
	{
	case FIRST_PERSON_CAMERA:
		SetFriction(250.0f);
		//1인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다. 
		SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(FIRST_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	case SPACESHIP_CAMERA:
		SetFriction(125.0f);
		//스페이스 쉽 카메라일 때 플레이어에 중력이 작용하지 않는다. 
		SetGravity(XMFLOAT3(0.0f, 0.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(SPACESHIP_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.0f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 0.0f, 0.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	case THIRD_PERSON_CAMERA:
		SetFriction(250.0f);
		//3인칭 카메라일 때 플레이어에 y-축 방향으로 중력이 작용한다.
		SetGravity(XMFLOAT3(0.0f, -250.0f, 0.0f));
		SetMaxVelocityXZ(300.0f);
		SetMaxVelocityY(400.0f);
		m_pCamera = OnChangeCamera(THIRD_PERSON_CAMERA, nCurrentCameraMode);
		m_pCamera->SetTimeLag(0.25f);
		m_pCamera->SetOffset(XMFLOAT3(0.0f, 20.0f, -50.0f));
		m_pCamera->GenerateProjectionMatrix(1.01f, 50000.0f, ASPECT_RATIO, 60.0f);
		break;
	default:
		break;
	}

	m_pCamera->SetPosition(Vector3::Add(m_xmf3Position, m_pCamera->GetOffset()));
	Update(fTimeElapsed);

	return m_pCamera;
}

//플레이어 위치가 바뀔 때 마다 호출
void CTerrainPlayer::OnPlayerUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3PlayerPosition = GetPosition();
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pPlayerUpdatedContext;

	/*지형에서 플레이어의 현재 위치 (x, z)의 지형 높이(y)를 구한다. 그리고 플레이어 메쉬의 높이가 12이고 플레이어의
	중심이 직육면체의 가운데이므로 y 값에 메쉬의 높이의 절반을 더하면 플레이어의 위치가 된다.*/
	float fHeight = pTerrain->GetHeight(xmf3PlayerPosition.x, xmf3PlayerPosition.z) + 6.0f;

	/*플레이어의 위치 벡터의 y-값이 음수이면(예를 들어, 중력이 적용되는 경우) 플레이어의 위치 벡터의 y-값이 점점 작아지게 된다.
	이 때 플레이어의 현재 위치 벡터의 y 값이 지형의 높이(실제로 지형의 높이 + 6)보다 작으면 플레이어의 일부가 지형 아래에 있게 된다.
	이러한 경우를 방지하려면 플레이어의 속도 벡터의 y 값을 0으로 만들고 플레이어의 위치 벡터의 y-값을 지형의 높이(실제로 지형의 높이 + 6)로 설정한다.
	그러면 플레이어는 항상 지형 위에 있게 된다.*/
	if (xmf3PlayerPosition.y < fHeight)
	{
		XMFLOAT3 xmf3PlayerVelocity = GetVelocity();
		xmf3PlayerVelocity.y = 0.0f;
		SetVelocity(xmf3PlayerVelocity);
		xmf3PlayerPosition.y = fHeight;
		SetPosition(xmf3PlayerPosition);
	}
}

//플레이어 위치가 바뀔 때 마다 호출
void CTerrainPlayer::OnCameraUpdateCallback(float fTimeElapsed)
{
	XMFLOAT3 xmf3CameraPosition = m_pCamera->GetPosition();
	
	/*하이트 맵에서 카메라의 현재 위치 (x, z)에 대한 지형의 높이(y 값)를 구한다.
	이 값이 카메라의 위치 벡터의 y-값 보다 크면 카메라가 지형의 아래에 있게 된다.
	이렇게 되면 카메라가 지형을 뚫고 들어가서, 일부 지형이 그려지지 않는 경우가 발생한다(카메라가 지형 안에 있으므로 삼각형의 와인딩 순서가 바뀐다).
	이러한 경우가 발생하지 않도록 카메라의 위치 벡터의 y-값의 최소값은 (지형의 높이 + 5)로 설정한다.
	카메라의 위치 벡터의 y-값의 최소값은 지형의 모든 위치에서 카메라가 지형 아래에 위치하지 않도록 설정해야 한다.*/
	CHeightMapTerrain* pTerrain = (CHeightMapTerrain*)m_pCameraUpdatedContext;
	float fHeight = pTerrain->GetHeight(xmf3CameraPosition.x, xmf3CameraPosition.z) + 5.0f;

	if (xmf3CameraPosition.y <= fHeight)
	{
		xmf3CameraPosition.y = fHeight;
		m_pCamera->SetPosition(xmf3CameraPosition);
		if (m_pCamera->GetMode() == THIRD_PERSON_CAMERA)
		{
			//3인칭 카메라의 경우 카메라 위치(y-좌표)가 변경되었으므로 카메라가 플레이어를 바라보도록 한다.
			CThirdPersonCamera *p3rdPersonCamera = (CThirdPersonCamera *)m_pCamera.get();
			p3rdPersonCamera->SetLookAt(GetPosition());
		}
	}
}