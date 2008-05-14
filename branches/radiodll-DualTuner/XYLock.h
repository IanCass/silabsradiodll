#ifndef XYLOCK_H
#define XYLOCK_H

#include "XYCriticalSection.h"

class XYLock
{
	XYCriticalSection* m_pCS;
public:
	XYLock(XYCriticalSection* pCS)
	{
		m_pCS = pCS;
		if(m_pCS) m_pCS->Enter();
	}
	~XYLock()
	{
		if(m_pCS) m_pCS->Leave();
	}
};

#endif // XYLOCK_H