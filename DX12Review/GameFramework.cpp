#include "pch.h"
#include "GameFramework.h"
#include "Camera.h"

CGameFramework::CGameFramework()
{
	m_nRtvDescIncrementSize = 0;

	m_nDsvDescIncrementSize = 0;

	m_hFenceEvent = NULL;
	for (int i = 0; i < m_nSwapChainBuffers; ++i) m_nFenceValues[i] = 0;

	m_nWndClientWidth = Config::FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = Config::FRAME_BUFFER_HEIGHT;

	_tcscpy_s(m_pszFrameRate, _T("DX12 Review ("));
}

CGameFramework::~CGameFramework()
{
}

bool CGameFramework::OnCreate(HINSTANCE hInstance, HWND hMainWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//D3D 디바이스, Cmd큐/리스트, 스왑체인 등을 생성
	CreateD3DDevice();
	CreateCmdQueueAndList();
	CreateRtvAndDsvDescHeaps();
	CreateSwapChain();
	CreateDSV();

	//렌더링할 게임 객체 생성
	BuildObjects();

	return true;
}

void CGameFramework::OnDestroy()
{
	//GPU의 명령리스트 실행 대기
	WaitForGpuComplete();

	//게임 객체 소멸
	ReleaseObjects();

	::CloseHandle(m_hFenceEvent);

	m_pdxgiSwapChain->SetFullscreenState(FALSE, NULL);

#if defined(_DEBUG)
	//미소멸 객체가 존재하는지 추적
	ComPtr<IDXGIDebug1> pdxgiDebug;
	DXGIGetDebugInterface1(0, IID_PPV_ARGS(pdxgiDebug.GetAddressOf()));
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_DETAIL);
#endif
}

void CGameFramework::CreateSwapChain()
{
	//클라이언트 크기 구하기
	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	//스왑체인 세팅
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.BufferDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.BufferDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.OutputWindow = m_hWnd;
	//FLIP_DISCARD 스왑체인은 MSAA 직접 지원 X
	//dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	//dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	dxgiSwapChainDesc.SampleDesc.Count = 1;
	dxgiSwapChainDesc.SampleDesc.Quality = 0;
	dxgiSwapChainDesc.Windowed = TRUE;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//스왑체인 생성
	ComPtr<IDXGISwapChain> SwapChain;
	HRESULT hr = m_pdxgiFactory->CreateSwapChain(m_pd3dCmdQueue.Get(), &dxgiSwapChainDesc, SwapChain.GetAddressOf());

	if (FAILED(hr))
	{
		OutputDebugString(L"Swap Chain Creation Failed\n");
	}

	hr = SwapChain.As(&m_pdxgiSwapChain);

	if (FAILED(hr))
	{
		OutputDebugString(L"Swap Chain Casting Failed\n");
	}

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	//Alt + Enter로 전체화면 전환 비활성화
	hr = m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);

	if (FAILED(hr))
	{
		OutputDebugString(L"MWA NO ALT ENTER Setting Failed\n");
	}

#ifndef _WITH_SWAPCHAIN_FULLSCREEN_STATE
	CreateRTV();
#endif
}

void CGameFramework::CreateRtvAndDsvDescHeaps()
{
	//RTV 디스크립터 힙에 대한 정보
	D3D12_DESCRIPTOR_HEAP_DESC d3dDescHeapDesc;
	::ZeroMemory(&d3dDescHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	d3dDescHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescHeapDesc.NodeMask = 0;

	//RTV 디스크립터 힙 생성
	HRESULT hr = m_pd3dDevice->CreateDescriptorHeap(&d3dDescHeapDesc, IID_PPV_ARGS(m_pd3dRtvDescHeap.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"RTV Descriptor Heap Cration Failed");
	}
	//RTV 디스크립터 힙의 개별 원소 크기를 저장함
	m_nRtvDescIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);



	//DSV 디스크립터 힙에 대한 정보
	d3dDescHeapDesc.NumDescriptors = 1;
	d3dDescHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;

	//DSV 디스크립터 힙 생성
	hr = m_pd3dDevice->CreateDescriptorHeap(&d3dDescHeapDesc, IID_PPV_ARGS(m_pd3dDsvDescHeap.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"DSV Descriptor Heap Cration Failed");
	}
	//DSV 디스크립터 힙의 개별 원소 크기를 저장함
	m_nDsvDescIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
}

void CGameFramework::CreateD3DDevice()
{
	HRESULT hr;
	
	UINT nDXGIFactoryFlags = 0;

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> pd3dDebugController;
	hr = D3D12GetDebugInterface(IID_PPV_ARGS(pd3dDebugController.GetAddressOf()));
	if (pd3dDebugController)
	{
		pd3dDebugController->EnableDebugLayer();
	}
	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

	hr = ::CreateDXGIFactory2(nDXGIFactoryFlags, IID_PPV_ARGS(m_pdxgiFactory.GetAddressOf()));
	
	if (FAILED(hr))
	{
		OutputDebugString(L"DXGI Factory Creation Failed\n");
	}

	//하드웨어 어댑터 열거 및 D3D 12.0 지원하는 하드웨어 디바이스 생성
	ComPtr<IDXGIAdapter1> pd3dAdapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, pd3dAdapter.GetAddressOf()); ++i)
	{
		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_pd3dDevice.GetAddressOf()))))
		{
			break;
		}
		else
		{
			OutputDebugString(L"D3D12 Device Creation Failed\n");
		}
	}

	//하드웨어 디바이스 생성 실패 시 WARP 디바이스 생성
	if (!pd3dAdapter)
	{
		m_pdxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(pd3dAdapter.GetAddressOf()));
		D3D12CreateDevice(pd3dAdapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(m_pd3dDevice.GetAddressOf()));
	}

	//다중샘플링 품질 수준 체크
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualiityLevels;
	d3dMsaaQualiityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualiityLevels.SampleCount = 4;	//Msaa4x 다중샘플링
	d3dMsaaQualiityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualiityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualiityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xQualityLevels = d3dMsaaQualiityLevels.NumQualityLevels;
	//품질수준 1 이상인 경우 MSAA 활성화
	m_bMsaa4xEnable = (m_nMsaa4xQualityLevels) ? true : false;

	//펜스 생성(기본, CPU와 GPU 동기화 목적 객체). 초기값은 0으로
	hr = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pd3dFence.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"Fence Creation Failed\n");
	}

	//펜스 동기화를 위한 이벤트 객체 생성. 초기값 FALSE, 이벤트 실행 시 이벤트 값을 자동 FALSE로 설정.
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
}

void CGameFramework::CreateCmdQueueAndList()
{
	//Command Queue(Direct type = GPU 직접 실행) 생성
	D3D12_COMMAND_QUEUE_DESC d3dCmdQueueDesc;
	::ZeroMemory(&d3dCmdQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCmdQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	d3dCmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

	HRESULT hr = m_pd3dDevice->CreateCommandQueue(&d3dCmdQueueDesc, IID_PPV_ARGS(m_pd3dCmdQueue.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"Command Queue Creation Failed\n");
	}

	//Command Allocator 생성. GPU 명령을 저장하기 위한 메모리 공간을 할당
	hr = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(m_pd3dCmdAllocator.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"Command Allocator Creation Failed\n");
	}

	//파이프라인 상태 객체(PSO)는 생성 시점에는 지정하지 않음. 추후 명령 기록 전에 PSO를 설정해주면 됨.
	hr = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_pd3dCmdAllocator.Get(), NULL, IID_PPV_ARGS(m_pd3dCmdList.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"Command List Creation Failed\n");
	}

	//생성 직후 Open 상태인 명령리스트를 Close.
	hr = m_pd3dCmdList->Close();
}

//스왑체인의 각 후면 버퍼에 대해 RTV 디스크립터 생성
void CGameFramework::CreateRTV()
{	
	//RTV 디스크립터 힙의 첫 번째 디스크립터 슬롯에 대한 CPU 핸들 가져오기
	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	HRESULT hr;
	for (UINT i = 0; i < m_nSwapChainBuffers; ++i)
	{
		//i번째 후면 버퍼 리소스에 대한 ID3D12Resource 인터페이스를 가져옴
		hr = m_pdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(m_ppd3dSwapChainBackBuffers[i].ReleaseAndGetAddressOf()));
		if (FAILED(hr))
		{
			OutputDebugString(L"Swapchain GetBuffer Failed\n");
			continue;
		}
		//위에서 가져온 후면 버퍼 리소스를 렌더 타깃으로 사용할 수 있도록
		//현재 RTV 디스크립터 슬롯에 해당 리소스에 대한 RTV 디스크립터를 기록함
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dSwapChainBackBuffers[i].Get(), NULL, d3dRtvCPUDescHandle);
		//다음 디스크립터 슬롯으로 넘어가기
		d3dRtvCPUDescHandle.ptr += m_nRtvDescIncrementSize;
	}
}

void CGameFramework::CreateDSV()
{
	//Committed 리소스 생성 세팅
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1;
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;

	//FLIP_DISCARD 스왑체인은 MSAA 직접 지원 X
	//d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	//d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xQualityLevels - 1) : 0;
	d3dResourceDesc.SampleDesc.Count = 1;
	d3dResourceDesc.SampleDesc.Quality = 0;

	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	//실제로 데이터가 저장될 힙 세팅
	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;

	//초기화값
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0;

	//리소스 생성
	HRESULT hr = m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, IID_PPV_ARGS(m_pd3dDepthStencilBuffer.GetAddressOf()));
	if (FAILED(hr))
	{
		OutputDebugString(L"Depth Stencil Resource Creation Failed\n");
	}

	//깊이스텐실 버퍼 뷰 생성
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescHandle = m_pd3dDsvDescHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer.Get(), NULL, d3dDsvCPUDescHandle);
}

void CGameFramework::BuildObjects()
{
	m_pd3dCmdList->Reset(m_pd3dCmdAllocator.Get(), NULL);

	//카메라 객체를 생성하여 뷰포트, 씨저 사각형, 투영 변환 행렬, 카메라 변환 행렬을 생성하고 설정한다.
	m_pCamera = std::make_shared<CCamera>();
	m_pCamera->SetViewport(0, 0, m_nWndClientWidth, m_nWndClientHeight, 0.0f, 1.0f);
	m_pCamera->SetScissorRect(0, 0, m_nWndClientWidth, m_nWndClientHeight);
	m_pCamera->GenerateProjectionMatrix(1.0f, 500.0f, float(m_nWndClientWidth) / float(m_nWndClientHeight), 90.0f);
	m_pCamera->GenereateViewMatrix(XMFLOAT3(0.0f, 15.0f, -25.0f), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(0.0f, 1.0f, 0.0f));

	//씬 객체를 생성하고 씬에 포함될 게임 객체들을 생성한다.
	m_pScene = std::make_unique<CScene>();
	m_pScene->BuildObjects(m_pd3dDevice.Get(), m_pd3dCmdList.Get());

	//씬 객체를 생성하기 위하여 필요한 그래픽 명령 리스트들을 명령 큐에 추가한다.
	m_pd3dCmdList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCmdList.Get()};
	m_pd3dCmdQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//그래픽 명령 리스트들이 모두 실행될 때까지 기다린다.
	WaitForGpuComplete();

	//그래픽 리소스들을 생성하는 과정에 생성된 업로드 버퍼들을 소멸시킨다.
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();
}

void CGameFramework::ReleaseObjects()
{
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) m_pScene.reset();
}

void CGameFramework::ProcessInput()
{
}

void CGameFramework::AnimateObjects()
{
	if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
}

void CGameFramework::FrameAdvance()
{
	//타이머의 시간이 갱신되도록 하고 프레임 레이트를 계산한다.
	m_GameTimer.Tick(0.0f);

	ProcessInput();

	AnimateObjects();

	HRESULT hr = m_pd3dCmdAllocator->Reset();
	if (FAILED(hr))
	{
		OutputDebugString(L"Command Allocator Reset Failed in FrameAdvance()\n");
	}
	hr = m_pd3dCmdList->Reset(m_pd3dCmdAllocator.Get(), NULL);
	if (FAILED(hr))
	{
		OutputDebugString(L"Command List Reset Failed in FrameAdvance()\n");
	}

	// 현재 후면 버퍼 상태를 Present에서 렌더 타겟 상태로 바꿈.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dSwapChainBackBuffers[m_nSwapChainBufferIndex].Get();
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCmdList->ResourceBarrier(1, &d3dResourceBarrier);


	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
	//현재 렌더 타겟에 해당하는 RTV 디스크립터 힙의 CPU 핸들을 계산
	d3dRtvCPUDescHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescIncrementSize);

	//DSV 디스크립터 힙 CPU 핸들을 가져온 뒤 원하는 값으로 지움
	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescHandle = m_pd3dDsvDescHeap->GetCPUDescriptorHandleForHeapStart();

	//RTV와 DSV를 OM(Output Merger)에 연결
	m_pd3dCmdList->OMSetRenderTargets(1, &d3dRtvCPUDescHandle, TRUE, &d3dDsvCPUDescHandle);

	//색상 지정하여 렌더 타겟을 지움
	float pfClearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
	m_pd3dCmdList->ClearRenderTargetView(d3dRtvCPUDescHandle, pfClearColor/*Colors::Azure*/, 0, NULL);


	m_pd3dCmdList->ClearDepthStencilView(d3dDsvCPUDescHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	//렌더링 코드는 이 공간에 추가
	if (m_pScene) m_pScene->Render(m_pd3dCmdList.Get(), m_pCamera.get());

	// 현재 후면 버퍼 상태를 렌더 타겟에서 Present로 바꿈.
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCmdList->ResourceBarrier(1, &d3dResourceBarrier);

	//Cmd 리스트 닫기
	hr = m_pd3dCmdList->Close();
	if (FAILED(hr))
	{
		OutputDebugString(L"Command List Closing Failed\n");
	}

	//Cmd 리스트를 Cmd 큐에 추가 및 실행
	ID3D12CommandList* ppd3dCmdLists[] = { m_pd3dCmdList.Get()};
	m_pd3dCmdQueue->ExecuteCommandLists(1, ppd3dCmdLists);

	//GPU가 모든 Cmd 리스트를 실행할 때 까지 대기
	WaitForGpuComplete();

	//스왑체인 Present. Present 후 현재 렌더 타겟(후면 버퍼)의 내용이 전면버퍼로 옮겨지고 렌더 타겟 인덱스가 바뀜.
	m_pdxgiSwapChain->Present(0, 0);

	//Present1 호출 후, 스왑체인 인터페이스 객체는 SwapEffect가 FLIP_DISCARD이므로
	//알아서 후면 버퍼 인덱스를 바꿈. 해당 인덱스 값을 가져와서 CPU 변수인 스왑체인 버퍼 인덱스에 저장.
	MoveToNextFrame();

	m_GameTimer.GetFrameRate(m_pszFrameRate + 13, 36);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFramework::WaitForGpuComplete()
{
	//CPU 펜스 값 증가
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	
	//GPU가 펜스 값을 설정하는 명령을 명령 큐에 추가함
	HRESULT hr = m_pd3dCmdQueue->Signal(m_pd3dFence.Get(), nFenceValue);
	if (FAILED(hr))
	{
		OutputDebugString(L"Fence Signal Command Failed\n");
	}
	else
	{
		//펜스의 현재 값이 설정 값보다 작으면, 설정 값이 될 때까지 기다림
		if (m_pd3dFence->GetCompletedValue() < nFenceValue)
		{
			hr = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
			if (FAILED(hr))
			{
				OutputDebugString(L"Fence Event Set Failed\n");
			}
			::WaitForSingleObject(m_hFenceEvent, INFINITE);
		}
	}
}

void CGameFramework::ChangeSwapChainState()
{
	WaitForGpuComplete();

	BOOL bFullScreenState = FALSE;
	HRESULT hr = m_pdxgiSwapChain->GetFullscreenState(&bFullScreenState, NULL);
	if (FAILED(hr))
	{
		OutputDebugString(L"GetFullscreenState Failed\n");
	}

	hr = m_pdxgiSwapChain->SetFullscreenState(!bFullScreenState, NULL);
	if (FAILED(hr))
	{
		OutputDebugString(L"SetFullscreenState Failed\n");
	}

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;

	hr = m_pdxgiSwapChain->ResizeTarget(&dxgiTargetParameters);
	if (FAILED(hr))
	{
		OutputDebugString(L"ResizeTarget Failed\n");
	}

	for (int i = 0; i < m_nSwapChainBuffers; ++i)
		if (m_ppd3dSwapChainBackBuffers[i]) m_ppd3dSwapChainBackBuffers[i].Reset();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	hr = m_pdxgiSwapChain->GetDesc(&dxgiSwapChainDesc);
	if (FAILED(hr))
	{
		OutputDebugString(L"GetDesc Failed\n");
	}

	hr = m_pdxgiSwapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth, m_nWndClientHeight, dxgiSwapChainDesc.BufferDesc.Format, dxgiSwapChainDesc.Flags);
	if (FAILED(hr))
	{
		OutputDebugString(L"ResizeBuffers Failed\n");
	}

	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	CreateRTV();
}

void CGameFramework::MoveToNextFrame()
{
	m_nSwapChainBufferIndex = m_pdxgiSwapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hr = m_pd3dCmdQueue->Signal(m_pd3dFence.Get(), nFenceValue);
	if (m_pd3dFence->GetCompletedValue() < nFenceValue)
	{
		hr = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFramework::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		break;
	case WM_MOUSEMOVE:
		break;
	default:
		break;
	}
}

void CGameFramework::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYUP:
		switch (wParam)
		{
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			//윈도우 모드와 전체화면 모드 전환
			ChangeSwapChainState();
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
}

LRESULT CGameFramework::OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_SIZE:
	{
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return 0;
}

