#include "Timer.h"



Timer::Timer() : m_SecondsPerCount(0.0), m_DeltaTime(-1.0), m_BaseTime(0), m_PausedTime(0), m_PrevTime(0), m_CurrentTime(0), m_Stopped(false)
{
	__int64 CountsPerSecond;
	QueryPerformanceFrequency((LARGE_INTEGER*)&CountsPerSecond);
	m_SecondsPerCount = 1.0 / (double)CountsPerSecond;
}

void Timer::Tick()
{
	if (m_Stopped)
	{
		m_DeltaTime = 0.0;
		return;
	}

	__int64 CurrentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);
	m_CurrentTime = CurrentTime;

	m_DeltaTime = (m_CurrentTime - m_PrevTime) * m_SecondsPerCount;
	m_PrevTime = m_CurrentTime;

	if (m_DeltaTime < 0.0)
		m_DeltaTime = 0.0;
}

float Timer::DeltaTime() const
{
	return (float)m_DeltaTime;
}

void Timer::Reset()
{
	__int64 CurrentTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);

	m_BaseTime = CurrentTime;
	m_PrevTime = CurrentTime;
	m_StopTime = 0;
	m_Stopped = false;
}

void Timer::Stop()
{
	if (!m_Stopped)
	{
		__int64 CurrentTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&CurrentTime);

		m_StopTime = CurrentTime;
		m_Stopped = true;
	}
}

void Timer::Start()
{
	__int64 StartTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&StartTime);

	if (m_Stopped)
	{
		m_PausedTime += (StartTime - m_StopTime);
		m_PrevTime = StartTime;
		m_StopTime = 0;
		m_Stopped = false;
	}
}

float Timer::Time()const
{
	if (m_Stopped)
		return (float)(((m_StopTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
	else
		return (float)(((m_CurrentTime - m_PausedTime) - m_BaseTime) * m_SecondsPerCount);
}