#ifndef __STDAFX_H_
#define __STDAFX_H_

#include <list>
using namespace std;

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "commondefine.h"

#define LONG long
#define HANDLE unsigned long
#define LPSTR char *
#define LPTSTR char *
#define LPCSTR const char *
#define LPCTSTR const char *
#define LPVOID void *
#define PVOID void *
#define BOOL int
#define TRUE 1
#define FALSE 0
#define PBYTE unsigned char *

#define FILE_CURRENT	1
#define FILE_END		2
#define FILE_BEGIN		0

#ifndef ZeroMemory
#define ZeroMemory(x,y) memset(x, 0, y)
#endif


#endif 
