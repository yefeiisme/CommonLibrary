#include "stdafx.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include "KFilePath.h"


#ifndef WIN32
#include <unistd.h>
#else
#include <direct.h>
#endif
#include <string.h>
//---------------------------------------------------------------------------
#ifdef WIN32
static char szRootPath[MAXPATH] = "C:";		// 启动路径
static char szCurrPath[MAXPATH] = "\\";		// 当前路径
#else
static char szRootPath[MAXPATH] = "/";		// 启动路径
static char szCurrPath[MAXPATH] = "/";		// 当前路径
#endif

int RemoveTwoPointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	//KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\..\\";
#else
	LPCTSTR lpszOld = "/../";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 3;
		while(lpszTarget > szPath)
		{
			lpszTarget--;
			if ((*lpszTarget) == '\\' ||(*lpszTarget) == '/')
				break;
		}
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveTwoPointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveOnePointPath(LPTSTR szPath, int nLength)
{
	int nRemove = 0;
	KASSERT(szPath);
#ifdef WIN32
	LPCTSTR lpszOld = "\\.\\";
#else
	LPCTSTR lpszOld = "/./";
#endif
	LPTSTR lpszTarget = strstr(szPath, lpszOld);
	if (lpszTarget)
	{
		LPTSTR lpszAfter = lpszTarget + 2;
		memmove(lpszTarget, lpszAfter, (nLength - (lpszAfter - szPath) + 1) * sizeof(char));
		nRemove = (lpszAfter - lpszTarget);
		return RemoveOnePointPath(szPath, nLength - nRemove);
	}

	return nLength - nRemove;
}

int RemoveAllPointPath(LPTSTR szPath, int nLength)
{
	return RemoveOnePointPath(szPath, RemoveTwoPointPath(szPath, nLength));
}

//---------------------------------------------------------------------------
// 函数:	SetRootPath
// 功能:	设置程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
void g_SetRootPath(char *pstrPathName)
{
	if (pstrPathName)
	{
		g_DebugLog("set path %s\n", pstrPathName);
		g_StrCpy(szRootPath, pstrPathName);
	}
	else
	{
		memset( szRootPath, 0, sizeof( szRootPath ) );

		getcwd( szRootPath, MAXPATH );
	}

	// 去掉路径末尾的 '\'
	int len = g_StrLen(szRootPath);
	g_DebugLog("set path %s(%d)\n", szRootPath, len);
	if (szRootPath[len - 1] == '\\' || szRootPath[len - 1] == '/')
	{
		szRootPath[len - 1] = 0;
	}
	g_DebugLog("RootPath = %s", szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	GetRootPath
// 功能:	取得程序的根路径
// 参数:	lpPathName	路径名
// 返回:	void
//---------------------------------------------------------------------------
void g_GetRootPath(char *pstrPathName)
{
	g_StrCpy(pstrPathName, szRootPath);
}
//---------------------------------------------------------------------------
// 函数:	GetFullPath
// 功能:	取得文件的全路径名
// 参数:	lpPathName	路径名
//			lpFileName	文件名
// 返回:	void
//---------------------------------------------------------------------------
void g_GetFullPath(char *pstrPathName, char *pstrFileName)
{
	// 文件带有全路径
	if (pstrFileName[1] == ':')
	{
		g_StrCpy(pstrPathName, pstrFileName);
		return;
	}

	// 文件带有部分路径
	if (pstrFileName[0] == '\\' || pstrFileName[0] == '/')
	{
		g_StrCpy(pstrPathName, szRootPath);
		g_StrCat(pstrPathName, pstrFileName);
		return;
	}
	
	// 当前路径为全路径
#ifdef WIN32
	if (szCurrPath[1] == ':')
	{
		g_StrCpy(pstrPathName, szCurrPath);
		g_StrCat(pstrPathName, pstrFileName);
		return;
	}
#endif
	// 当前路径为部分路径
	g_StrCpy(pstrPathName, szRootPath);
        if(szCurrPath[0] != '\\' && szCurrPath[0] != '/') {
#ifdef WIN32
	g_StrCat(pstrPathName, "\\");
#else
	g_StrCat(pstrPathName, "/");
#endif
      }
	g_StrCat(pstrPathName, szCurrPath);

	if (pstrFileName[0] == '.' && (pstrFileName[1] == '\\'||pstrFileName[1] == '/') )
		g_StrCat(pstrPathName, pstrFileName + 2);
	else
		g_StrCat(pstrPathName, pstrFileName);
}
