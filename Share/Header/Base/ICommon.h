#ifndef __I_COMMON_H_
#define __I_COMMON_H_

class IRandom
{
public:
	virtual void			SetRandomSeed(const unsigned int uRandomSeed) = 0;
	virtual unsigned int	Random(const unsigned int uMax) = 0;
	virtual void			Release() = 0;
};

IRandom	*CreateRandom();

#endif
