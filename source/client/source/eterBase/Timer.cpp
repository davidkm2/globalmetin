#include "StdAfx.h"
#include "Timer.h"

static LARGE_INTEGER gs_liTickCountPerSec;
static DWORD gs_dwBaseTime=0;
static DWORD gs_dwServerTime=0;
static DWORD gs_dwClientTime=0;
static DWORD gs_dwFrameTime=0;

BOOL ELTimer_Init()
{
	gs_dwBaseTime = timeGetTime();
	return 1;
}

DWORD ELTimer_GetMSec()
{
	return timeGetTime() - gs_dwBaseTime; 
}

VOID ELTimer_SetServerMSec(DWORD dwServerTime)
{
	if (0 != dwServerTime) 
	{
		gs_dwServerTime = dwServerTime;
		gs_dwClientTime = CTimer::Instance().GetCurrentMillisecond();
	}
}

DWORD ELTimer_GetServerMSec()
{
	return CTimer::Instance().GetCurrentMillisecond() - gs_dwClientTime + gs_dwServerTime;
}

DWORD ELTimer_GetFrameMSec()
{
	return gs_dwFrameTime;
}

DWORD ELTimer_GetServerFrameMSec()
{
	return ELTimer_GetFrameMSec() - gs_dwClientTime + gs_dwServerTime;
}

VOID ELTimer_SetFrameMSec()
{
	gs_dwFrameTime = ELTimer_GetMSec();
}

CTimer::CTimer()
{
	ELTimer_Init();

	if (this) 
	{
		m_dwCurrentTime = 0;
		m_bUseRealTime = true;
		m_index = 0;

		m_dwElapsedTime = 0;

		m_fCurrentTime = 0.0f;
	}
}

CTimer::~CTimer()
{
}

void CTimer::SetBaseTime()
{
	m_dwCurrentTime = 0;
}

void CTimer::Advance()
{
	if (!m_bUseRealTime)
	{
		++m_index;

		if (m_index == 1)
			m_index = -1;

		m_dwCurrentTime += 16 + (m_index & 1);
		m_fCurrentTime = m_dwCurrentTime / 1000.0f;
	}
	else
	{
		DWORD currentTime = ELTimer_GetMSec();

		if (m_dwCurrentTime == 0)
			m_dwCurrentTime = currentTime;

		m_dwElapsedTime = currentTime - m_dwCurrentTime;
		m_dwCurrentTime = currentTime;
	}
}

void CTimer::Adjust(int iTimeGap)
{
	m_dwCurrentTime += iTimeGap;
}

float CTimer::GetCurrentSecond()
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec() / 1000.0f;

	return m_fCurrentTime;
}

DWORD CTimer::GetCurrentMillisecond()
{
	if (m_bUseRealTime)
		return ELTimer_GetMSec();

	return m_dwCurrentTime;
}

float CTimer::GetElapsedSecond()
{
	return GetElapsedMilliecond() / 1000.0f;
}

DWORD CTimer::GetElapsedMilliecond()
{
	if (!m_bUseRealTime)
		return 16 + (m_index & 1);

	return m_dwElapsedTime;
}

void CTimer::UseCustomTime()
{
	m_bUseRealTime = false;
}
