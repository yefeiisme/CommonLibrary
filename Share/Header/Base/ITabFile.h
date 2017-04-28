#ifndef __I_TABFILE_H__
#define __I_TABFILE_H__

class ITabFile
{
public:
	virtual int		GetHeight() const = 0;
	virtual bool	GetString(const int nRow, char *pstrColumn, char *pstrDefault, char *pstrRString, const unsigned long dwSize) = 0;
	virtual bool	GetString(const int nRow, const int nColumn, char *pstrDefault, char *pstrRString, const unsigned long dwSize) = 0;
	virtual bool	GetInteger(const int nRow, char *pstrColumn, const int nDefault, int *pnValue) = 0;
	virtual bool	GetInteger(const int nRow, const int nColumn, const int nDefault, int *pnValue) = 0;
	virtual bool	GetFloat(const int nRow, char *pstrColumn, const float fDefault, float *pfValue)	= 0;
	virtual bool	GetFloat(const int nRow, const int nColumn, const float fDefault, float *pfValue) = 0;
	virtual void	Release() = 0;
};

ITabFile *OpenTabFile(const char *FileName);

void	g_SetRootPath(char *pstrPathName);
void	g_GetRootPath(char *pstrPathName);
void	g_GetFullPath(char *pstrPathName, char *pstrFileName);

#endif
