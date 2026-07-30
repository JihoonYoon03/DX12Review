#pragma once

#include "Timer.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	// 프레임워크 초기화 함수
	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	//스왑체인, 디바이스, DescHeap(RTV, DSV), Cmd큐/할당자/리스트 생성 함수
	void CreateSwapChain();
	void CreateRtvAndDsvDescHeaps();
	void CreateD3DDevice();
	void CreateCmdQueueAndList();

	//렌더타겟뷰, 깊이스텐실뷰 생성
	void CreateRTV();
	void CreateDSV();

	//렌더링할 메쉬, 게임 객체를 생성 및 소멸하는 함수
	void BuildObjects();
	void ReleaseObjects();

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	//CPU와 GPU를 동기화하는 함수
	void WaitForGpuComplete();

	// 윈도우 메시지(키보드, 마우스 입력)를 처리하는 함수
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

private:
	//현재 실행중인 프로그램 모듈 식별 핸들 HINSTANCE, 특정 윈도우 창 식별 핸들 HWND
	HINSTANCE	m_hInstance;
	HWND		m_hWnd;

	//클라이언트 폭, 높이
	int			m_nWndClientWidth;
	int			m_nWndClientHeight;

	//DXGI 팩토리 인터페이스 포인터
	ComPtr<IDXGIFactory4>	m_pdxgiFactory;
	//스왑 체인 인터페이스 포인터. 디스플레이 제어
	ComPtr<IDXGISwapChain3>	m_pdxgiSwapChain;
	//Direct3D Device 인터페이스 포인터. 리소스 생성 관리
	ComPtr<ID3D12Device>	m_pd3dDevice;

	//다중샘플링 활성화 여부, 레벨 설정
	bool					m_bMsaa4xEnable = false;
	UINT					m_nMsaa4xQualityLevels = 0;

	//스왑체인 후면 버퍼 개수, 현재 인덱스 번호
	static const UINT		m_nSwapChainBuffers = 2;
	UINT					m_nSwapChainBufferIndex;

	//스왑체인 후면 버퍼 = 렌더 타겟 버퍼
	ComPtr<ID3D12Resource>			m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	//Descriptor Heap(RTV) 인터페이스 포인터와 크기
	ComPtr<ID3D12DescriptorHeap>	m_pd3dRtvDescHeap;
	UINT							m_nRtvDescIncrementSize;

	//깊이스텐실 버퍼
	ComPtr<ID3D12Resource>			m_pd3dDepthStencilBuffer;
	//Descriptor Heap(DSV) 인터페이스 포인터와 크기
	ComPtr<ID3D12DescriptorHeap>	m_pd3dDsvDescHeap;
	UINT							m_nDsvDescIncrementSize;

	//Command 큐, 할당자, 리스트 인터페이스 포인터
	ComPtr<ID3D12CommandQueue>			m_pd3dCmdQueue;
	ComPtr<ID3D12CommandAllocator>		m_pd3dCmdAllocator;
	ComPtr<ID3D12GraphicsCommandList>	m_pd3dCmdList;

	//그래픽스 파이프라인 상태 객체 인터페이스 포인터
	ComPtr<ID3D12PipelineState>		m_pd3dPipelineState;

	//Fence 인터페이스 포인터, Fence의 값, 이벤트 핸들
	ComPtr<ID3D12Fence>				m_pd3dFence;
	UINT64							m_nFenceValue;
	HANDLE							m_hFenceEvent;

	//뷰포트와 ScissorRect 정보 구조체
	D3D12_VIEWPORT					m_d3dViewport;
	D3D12_RECT						m_d3dScissorRect;


	//다음은 게임 프레임워크에서 사용할 타이머이다.
	CGameTimer m_GameTimer;

	//다음은 프레임 레이트를 주 윈도우의 캡션에 출력하기 위한 문자열이다.
	_TCHAR m_pszFrameRate[50];
};

