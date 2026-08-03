#pragma once

class CGameTimer
{
public:
	CGameTimer();
	virtual ~CGameTimer();

	void Start() {}
	void Stop() {}
	void Reset();

	//타이머의 시간을 갱신
	void Tick(float fLockFPS = 0.0f);
	//프레임레이트를 반환
	unsigned long GetFrameRate(LPTSTR lpszString = NULL, int nCharacters = 0);
	//프레임의 평균 경과 시간을 반환
	float GetTimeElapsed();

private:
	//컴퓨터가 Performance Counter를 가지고 있는가
	bool m_bHardwareHasPerformanceCounter;
	
	//Scale Counter의 양
	float m_fTimeScale;
	//마지막 프레임 이후 지나간 시간
	float m_fTimeElapsed;

	//현재 시간
	__int64 m_nCurrentTime;
	//마지막 프레임 시간
	__int64 m_nLastTime;
	//컴퓨터의 Performance Frequency
	__int64 m_nPerformanceFrequency;

	//프레임 시간을 누적하기 위한 배열
	float m_fFrameTime[Config::MAX_SAMPLE_COUNT];
	//누적 프레임 횟수
	ULONG m_nSampleCount;

	//현재 프레임 레이트
	unsigned long m_nCurrentFrameRate;
	//초당 프레임 수
	unsigned long m_nFramesPerSecond;
	//프레임 레이트 계산 소요 시간
	float m_fFPSTimeElapsed;

	bool m_bStopped;
};