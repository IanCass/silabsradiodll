#ifndef XYCRITICALSECTION_H
#define XYCRITICALSECTION_H

#include "windows.h"

class XYCriticalSection
{
	long m_nLockCount;
	long m_nThreadId;
	bool SetLock(const long nThreadId);
public:
	XYCriticalSection()
	{
		m_nThreadId = 0;
		m_nLockCount = 0;
	}
	void Enter();
	void Leave();
	bool Try();
};

#endif // XYCRITICALSECTION_H