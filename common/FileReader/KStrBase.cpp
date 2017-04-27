#include "stdafx.h"
#include "KDebug.h"
#include "KMemBase.h"
#include "KStrBase.h"
#include <string.h>

 int g_StrLen(LPCSTR lpStr)
{
#ifdef WIN32
	register int nLen;

	__asm
	{
		mov		edi, lpStr
		mov		ecx, 0xffffffff
		xor		al, al
		repne	scasb
		not		ecx
		dec		ecx
		mov		nLen, ecx
	}
	return nLen;
#else
	return strlen(lpStr);
#endif
}
//---------------------------------------------------------------------------
// 函数:	StrEnd
// 功能:	返回字符串结尾指针
// 参数:	lpStr	:	字符串开头的指针
// 返回:	lpEnd	:	字符串末尾的指针
//---------------------------------------------------------------------------
 LPSTR g_StrEnd(LPCSTR lpStr)
{
#ifdef WIN32
	register LPSTR lpEnd;

	__asm
	{
		mov		edi, lpStr
		mov		ecx, 0xffffffff
		xor		al, al
		repne	scasb
		lea		eax, [edi - 1]
		mov		lpEnd, eax
	}
	return lpEnd;
#else
	return (char *)lpStr + strlen(lpStr);
#endif
}
//---------------------------------------------------------------------------
// 函数:	StrCpy
// 功能:	字符串拷贝
// 参数:	lpDest	:	目标字符串
//			lpSrc	:	源字符串
// 返回:	void
//---------------------------------------------------------------------------
 void g_StrCpy(LPSTR lpDest, LPCSTR lpSrc)
{
#ifdef WIN32
	__asm
	{
		mov		edi, lpSrc
		mov		ecx, 0xffffffff
		xor		al, al
		repne	scasb
		not		ecx
		mov		edi, lpDest
		mov		esi, lpSrc
		mov		edx, ecx
		shr		ecx, 2
		rep		movsd
		mov		ecx, edx
		and		ecx, 3
		rep		movsb
	};
#else
	strcpy(lpDest, lpSrc);
#endif
}
//---------------------------------------------------------------------------
// 函数:	StrCpyLen
// 功能:	字符串拷贝,有最大长度限制
// 参数:	lpDest	:	目标字符串
//			lpSrc	:	源字符串
//			nMaxLen	:	最大长度
// 返回:	void
//---------------------------------------------------------------------------
 void g_StrCpyLen(LPSTR lpDest, LPCSTR lpSrc, const unsigned int uMaxLen)
{
#ifdef WIN32
	__asm
	{
		xor		al, al
		mov		edx, uMaxLen
		dec		edx
		jg		copy_section

		jl		finished
		mov		edi, lpDest
		stosb
		jmp		finished

copy_section:
		mov		edi, lpSrc
		mov		ecx, 0xffffffff
		repne	scasb
		not		ecx
		dec		ecx
		cmp		ecx, edx
		jle		loc_little_equal
		mov		ecx, edx

loc_little_equal:

		mov		edi, lpDest
		mov		esi, lpSrc
		mov		edx, ecx
		shr		ecx, 2
		rep		movsd
		mov		ecx, edx
		and		ecx, 3
		rep		movsb
		stosb
		
finished:
	};
#else
	strncpy(lpDest, lpSrc, uMaxLen);
#endif
}
//---------------------------------------------------------------------------
// 函数:	StrCat
// 功能:	字符串末尾追加另一个字符串
// 参数:	lpDest	:	目标字符串
//			lpSrc	:	源字符串
// 返回:	void
//---------------------------------------------------------------------------
 void g_StrCat(LPSTR lpDest, LPCSTR lpSrc)
{
	register LPSTR lpEnd;

	lpEnd = g_StrEnd(lpDest);
	g_StrCpy(lpEnd, lpSrc);
}
//---------------------------------------------------------------------------
// 函数:	StrCatLen
// 功能:	字符串末尾追加另一个字符串,有最大长度限制
// 参数:	lpDest	:	目标字符串
//			lpSrc	:	源字符串
//			nMaxLen	:	最大长度
// 返回:	void
//---------------------------------------------------------------------------
 void g_StrCatLen(LPSTR lpDest, LPCSTR lpSrc, int nMaxLen)
{
	register LPSTR lpEnd;

	lpEnd = g_StrEnd(lpDest);
	g_StrCpyLen(lpEnd, lpSrc, nMaxLen);
}
//---------------------------------------------------------------------------
// 函数:	StrCmp
// 功能:	字符串比较
// 参数:	lpDest	:	字符串1	
//			lpSrc	:	字符串2
// 返回:	TRUE	:	相同
//			FALSE	:	不同
//---------------------------------------------------------------------------
 BOOL g_StrCmp(LPCSTR lpDest, LPCSTR lpSrc)
{
	register int nLen1, nLen2;

	nLen1 = g_StrLen(lpDest);
	nLen2 = g_StrLen(lpSrc);
	if (nLen1 != nLen2)
		return FALSE;
	return g_MemComp((void*)lpDest, (void*)lpSrc, nLen1);
}
//---------------------------------------------------------------------------
// 函数:	StrCmpLen
// 功能:	字符串比较,限定长度
// 参数:	lpDest	:	字符串1	
//			lpSrc	:	字符串2
//			nLen	:	长度
// 返回:	TRUE	:	相同
//			FALSE	:	不同
//---------------------------------------------------------------------------
 BOOL g_StrCmpLen(LPCSTR lpDest, LPCSTR lpSrc, int nMaxLen)
{
	register int nLen1, nLen2;

	nLen1 = g_StrLen(lpDest);
	nLen2 = g_StrLen(lpSrc);
	if (nMaxLen > nLen1)
		nMaxLen = nLen1;
	if (nMaxLen > nLen2)
		nMaxLen = nLen2;
	return g_MemComp((void*)lpDest, (void*)lpSrc, nMaxLen);
}
