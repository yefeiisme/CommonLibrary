#ifndef KPakFile_H
#define KPakFile_H

#include "KFile.h"
#include "XPackFile.h"

class KPakFile
{
public:
	KPakFile();
	~KPakFile();
	bool			Open(const char* pszFileName);
	void			Close();
	unsigned int	Read(void* pBuffer, unsigned int uSize);
	unsigned int	Size();
	void			Release();
private:
	KFile			m_File;			// 真实文件(不在包中)
};

#endif
