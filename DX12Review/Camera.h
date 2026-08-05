#pragma once

constexpr float ASPECT_RATIO = (float)(Config::FRAME_BUFFER_WIDTH / Config::FRAME_BUFFER_HEIGHT);

//카메라 종류를 나타내는 상수
#define FIRST_PERSON_CAMERA 0x01
#define SPACESHIP_CAMERA	0x02
#define THIRD_PERSON_CAMERA 0x03

class CPlayer;

//Vertex Shader Constant Buffer Camera Info
struct VS_CB_CAMERA_INFO
{
	XMFLOAT4X4 m_xmf4x4View;
	XMFLOAT4X4 m_xmf4x4Projection;
};

class CCamera
{
public:
	CCamera();
	CCamera(CCamera* pCamera);
	virtual ~CCamera();

	//카메라 정보를 셰이더 프로그램에게 전달하기 위한 상수 버퍼 생성/갱신
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//카메라 변환 행렬을 생성한다
	void GenerateViewMatrix();
	void GenerateViewMatrix(XMFLOAT3 xmf3Position, XMFLOAT3 xmf3LookAt, XMFLOAT3 xmf3Up);
	//여러번 회전 시 누적되는 실수 연산 부정확성 때문에, 카메라의 로컬 좌표축들이 직교하지 않을 수 있음을 보정해주는 함수
	void RegenerateViewMatrix();

	//투영 변환 행렬을 생성한다
	void GenerateProjectionMatrix(float fNearPlaneDistance, float fFarPlaneDistance, float fAspectRatio, float fFOVAngle);

	void SetViewport(int xTopLeft, int yTopLeft, int nWidth, int nHeight, float fMinZ = 0.0f, float fMaxZ = 1.0f);
	void SetScissorRect(LONG xLeft, LONG yTop, LONG xRigth, LONG yBottom);

	virtual void SetViewportsAndScissorRects(ID3D12GraphicsCommandList* pd3dCommandList);

	void SetPlayer(CPlayer* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer* GetPlayer() { return m_pPlayer; }

	void SetMode(DWORD nMode) { m_nMode = nMode; }
	DWORD GetMode() { return m_nMode; }

	void SetPosition(XMFLOAT3 xmf3Position) { m_xmf3Position = xmf3Position; }
	XMFLOAT3& GetPosition() { return m_xmf3Position; }

	void SetLookAtPosition(XMFLOAT3 xmf3LookAtWorld) { m_xmf3LookAtWorld = xmf3LookAtWorld; }
	XMFLOAT3& GetLookAtPosition() { return m_xmf3LookAtWorld; }

	XMFLOAT3& GetRightVector() { return m_xmf3Right; }
	XMFLOAT3& GetUpVector() { return m_xmf3Up; }
	XMFLOAT3& GetLookVector() { return m_xmf3Look; }

	float& GetPitch() { return m_fPitch; }
	float& GetRoll() { return m_fRoll; }
	float& GetYaw() { return m_fYaw; }

	void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }
	XMFLOAT3& GetOffset() { return m_xmf3Offset; }

	void SetTimeLag(float fTimeLag) { m_fTimeLag = fTimeLag; }
	float GetTimeLag() { return m_fTimeLag; }

	XMFLOAT4X4 GetViewMatrix() { return m_xmf4x4View; }
	XMFLOAT4X4 GetProjectionMatrix() { return m_xmf4x4Projection; }
	D3D12_VIEWPORT GetViewport() { return m_d3dViewport; }
	D3D12_RECT GetScissorRect() { return m_d3dScissorRect; }

	//카메라를 xmf3Shift만큼 이동한다
	virtual void Move(const XMFLOAT3& xmf3Shift) { m_xmf3Position.x += xmf3Shift.x; m_xmf3Position.y += xmf3Shift.y; m_xmf3Position.z += xmf3Shift.z; }
	//카메라를 x, y, z축으로 회전하는 가상함수이다.
	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f) {}
	//카메라의 이동, 회전에 따라 카메라의 정보를 갱신하는 가상함수이다.
	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed) {}
	//3인칭 카메라에서 카메라가 바라보는 지점을 설정한다. 일반적으로 플레이어
	virtual void SetLookAt(XMFLOAT3 xmf3LookAt) {}

protected:
	//카메라 위치 벡터
	XMFLOAT3		m_xmf3Position;

	//카메라의 로컬 x, y, z축을 나타내는 벡터
	XMFLOAT3		m_xmf3Right;
	XMFLOAT3		m_xmf3Up;
	XMFLOAT3		m_xmf3Look;

	//카메라가 x, z, y축으로 회전한 각도
	float			m_fPitch;
	float			m_fRoll;
	float			m_fYaw;

	//카메라의 종류(1인칭, 우주선, 3인칭)
	DWORD			m_nMode;

	//플레이어가 바라볼 위치 벡터. 주로 3인칭에서 사용
	XMFLOAT3		m_xmf3LookAtWorld;
	//플레이어와 카메라의 오프셋을 나타내는 벡터이다. 주로 3인칭에서 사용
	XMFLOAT3		m_xmf3Offset;
	//플레이어가 회전할 때 얼마만큼의 시간 지연 후 회전시킬 것인가(Camera Lag)
	float			m_fTimeLag;

	//카메라 변환 행렬
	XMFLOAT4X4		m_xmf4x4View;
	//투영 변환 행렬
	XMFLOAT4X4		m_xmf4x4Projection;

	//뷰포트와 씨저 사각형
	D3D12_VIEWPORT	m_d3dViewport;
	D3D12_RECT		m_d3dScissorRect;

	//카메라를 갖는 플레이어에 대한 포인터
	CPlayer*		m_pPlayer;
};

class CSpaceShipCamera : public CCamera
{
public:
	CSpaceShipCamera(CCamera* pCamera);
	virtual ~CSpaceShipCamera() {}

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CFirstPersonCamera : public CCamera
{
public:
	CFirstPersonCamera(CCamera* pCamera);
	virtual ~CFirstPersonCamera() {}

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera : public CCamera
{
public:
	CThirdPersonCamera(CCamera* pCamera);
	virtual ~CThirdPersonCamera() {}

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};