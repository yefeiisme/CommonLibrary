#ifndef __SERVER_NETWORK_SELECT_H_
#define __SERVER_NETWORK_SELECT_H_

#include <list>
#include "../NetworkHead.h"

using namespace std;

class CServerNetwork : public IServerNetwork
{
private:
	pfnConnectEvent			m_pfnConnectCallBack;
	void					*m_pFunParam;

	CTcpConnection			*m_pListenLink;

	CTcpConnection			*m_pTcpConnection;		// 所有的网络连接
	CTcpConnection			**m_pFreeConn;			// 当前处理空闲状态的CNetLink索引数组

	unsigned int			m_uMaxConnCount;
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink的索引，类似list的iterator用法

	unsigned int			m_uSleepTime;

	fd_set					m_ReadSet;
	fd_set					m_ErrorSet;

	list<CTcpConnection*>	m_listActiveConn;
	list<CTcpConnection*>	m_listCloseWaitConn;

	bool					m_bRunning;
	bool					m_bExited;
private:
	inline CTcpConnection	*GetNewConnection()
	{
		return m_uFreeConnIndex >= m_uMaxConnCount ? NULL : m_pFreeConn[m_uFreeConnIndex++];
	}

	inline void				AddAvailableConnection(CTcpConnection *pConnection)
	{
		if (m_uFreeConnIndex)
		{
			m_pFreeConn[--m_uFreeConnIndex]	= pConnection;
		}
	}

	int						SetNoBlocking(CTcpConnection *pTcpConnection);
	void					AcceptClient(const SOCKET nNewSocket);

	void					DisconnectConnection(CTcpConnection *pTcpConnection);
	void					RemoveConnection(CTcpConnection *pTcpConnection);
	void					CloseConnection(CTcpConnection *pTcpConnection);

	void					NetworkAction();

	void					ReadAction();
	void					WriteAction();
	void					CloseAction();

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
public:
	CServerNetwork();
	~CServerNetwork();

	bool					Initialize(
										const unsigned short usPort,
										void *lpParam,
										pfnConnectEvent pfnConnectCallBack,
										const unsigned int uConnectionNum,
										const unsigned int uSendBufferLen,
										const unsigned int uRecvBufferLen,
										const unsigned int uTempSendBufferLen,
										const unsigned int uTempRecvBufferLen,
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
};

#endif
