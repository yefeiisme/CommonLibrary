#ifndef _ICLIENT_H_
#define _ICLIENT_H_

#include "INetwork.h"
#include "IRingBuffer.h"
#include <list>
using namespace std;

#define MAX_CONNECT_ADDR	128

struct SConnectRequest
{
	void			*pTarget;
	char			strAddr[MAX_CONNECT_ADDR];
	unsigned short	usPort;
};

class CTcpConnection;

class CClientNetwork : public IClientNetwork
{
private:
	CALLBACK_CLIENT_EVENT	m_pfnConnectCallBack;
	void					*m_pFunParam;

	CTcpConnection			*m_pTcpConnection;
	CTcpConnection			**m_pFreeConn;			// 当前处理空闲状态的CNetLink索引数组

	IRingBuffer				*m_pConnectBuffer;		// 用于接收连接请求的缓冲区

	unsigned int			m_uMaxConnCount;
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink的索引，类似list的iterator用法

	unsigned int			m_uSleepTime;

	list<CTcpConnection*>	m_listActiveConn;
	list<CTcpConnection*>	m_listWaitConnectedConn;
	list<CTcpConnection*>	m_listCloseWaitConn;

	fd_set					m_ReadSet;
	fd_set					m_WriteSet;
	fd_set					m_ErrorSet;

	bool					m_bRunning;
	bool					m_bExited;
public:
	CClientNetwork();
	~CClientNetwork();

	bool					Initialize(
										const unsigned int uClientCount,
										const unsigned int uSendBuffLen,
										const unsigned int uRecvBuffLen,
										const unsigned int uTempSendBuffLen,
										const unsigned int uTempRecvBuffLen,
										CALLBACK_CLIENT_EVENT pfnConnectCallBack,
										void *lpParm,
										const unsigned int uSleepTime
										);
	inline void				Stop()
	{
		m_bRunning	= false;
	}

	inline bool				IsExited()
	{
		return m_bExited;
	}

	void					Release();
	bool					ConnectTo(char *pstrAddr, const unsigned short usPort, void *pTarget);
private:
	inline void				AddAvailableConnection(CTcpConnection *pConnection)
	{
		if (m_uFreeConnIndex)
		{
			m_pFreeConn[--m_uFreeConnIndex]	= pConnection;
		}
	}

	void					TryConnect(const void *pPack);
	int						SetNoBlocking(CTcpConnection *pTcpConnection);
	void					RemoveConnection(CTcpConnection *pTcpConnection);

	void					ProcessConnectRequest();
	void					ProcessConnectedConnection();
	void					ProcessWaitConnectConnection();
	void					ProcessWaitCloseConnection();

	void					ThreadFunc();
	inline void				yield()
	{
#if defined (WIN32) || defined (WIN64)
		Sleep(m_uSleepTime);
#else
		struct timeval sleeptime;
		sleeptime.tv_sec	= 0;
		sleeptime.tv_usec	= m_uSleepTime * 1000;
		select(0, 0, 0, 0, &sleeptime);
#endif
	}
};

#endif
