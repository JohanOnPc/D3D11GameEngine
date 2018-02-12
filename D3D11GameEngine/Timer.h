#pragma once

#include <Windows.h>

class Timer
{
public:
	Timer(void);

	float Time(void)const;
	float DeltaTime(void)const;

	void Reset(void);
	void Start(void);
	void Stop(void);
	void Tick(void);

private:
	double  m_SecondsPerCount;
	double  m_DeltaTime;

	__int64 m_BaseTime;
	__int64 m_PausedTime;
	__int64 m_StopTime;
	__int64 m_PrevTime;
	__int64 m_CurrentTime;

	bool	m_Stopped;
};