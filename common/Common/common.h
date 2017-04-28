#ifndef __COMMON_H_
#define __COMMON_H_

#include "commondefine.h"
#include "ICommon.h"

class CRandom : public IRandom
{
public:
	CRandom();
	~CRandom();

	inline void			SetRandomSeed(const unsigned int uRandomSeed)
	{
		m_uRandomSeed	= uRandomSeed;
	}

	unsigned int		Random(const unsigned int uMax)
	{
		int64 f = m_uRandomSeed * 0x08088405 + 1;
		m_uRandomSeed = f;
		int64 t = f * uMax;
		t = t >> 32;
		return (unsigned int)t;
	}

	inline void			Release()
	{
		delete this;
	}

private:
	UINT					m_uRandomSeed;
};

#endif

