#ifndef __DAEMON_H_
#define __DAEMON_H_

#include "commondefine.h"

#if defined(WIN32) || defined(WIN64)
#include "windows.h"
typedef BOOL (WINAPI *OnQuitSignal)(DWORD CtrlType);
#elif defined(__linux)
typedef void (*OnQuitSignal)(int nSignal);
#elif defined(__APPLE__)
#endif

bool	DaemonInit(OnQuitSignal pfnQuitSignal);
void	yield(const unsigned int ulTime);
uint64	GetMicroTick();

#endif
