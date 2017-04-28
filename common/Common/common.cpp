#include "common.h"
#include <ctype.h>

CRandom::CRandom()
{
	m_uRandomSeed	= 42;
}

CRandom::~CRandom()
{
}

IRandom	*CreateRandom()
{
	CRandom	*pNewRandom	= new CRandom;
	if (nullptr == pNewRandom)
		return nullptr;

	return pNewRandom;
}
