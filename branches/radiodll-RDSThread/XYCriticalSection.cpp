#include "stdafx.h"
#include "XYCriticalSection.h"



bool XYCriticalSection::SetLock(const long nThreadId)
{
	if(::InterlockedCompareExchange((volatile LONG*)&m_nThreadId,(LONG)nThreadId,0)==0)
	{
		m_nLockCount = 1;
		return true;
	}
	return false;
}

void XYCriticalSection::Enter()
{
	long nThreadId = ::GetCurrentThreadId();
	if(m_nThreadId==nThreadId) m_nLockCount++;
	else
	{
		while(true)
		{
			if(SetLock(nThreadId)) break; 
			::Sleep(50);
		}
	}
}

void XYCriticalSection::Leave()
{
	long nThreadId = ::GetCurrentThreadId();
	if(m_nThreadId==nThreadId) 
	{
		if(m_nLockCount>1) m_nLockCount--;
		else
		{
			m_nLockCount = 0;
			::InterlockedExchange(&m_nThreadId,0);
		}
	}
}

bool XYCriticalSection::Try()
{
	long nThreadId = ::GetCurrentThreadId();
	if(m_nThreadId==nThreadId) 
	{
		m_nLockCount++;
		return true;
	}
	return SetLock(nThreadId);	
}
