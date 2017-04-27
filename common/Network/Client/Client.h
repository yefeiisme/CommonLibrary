#ifndef _ICLIENT_H_
#define _ICLIENT_H_

#include "INetwork.h"
#include <list>
using namespace std;

class CTcpConnection;

class CClientNetwork : public IClientNetwork
{
private:
	CTcpConnection			*m_pConnList;
	CTcpConnection			**m_pFreeConn;			// 当前处理空闲状态的CNetLink索引数组

	void					*m_pFunParam;

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
	ITcpConnection			*ConnectTo(char *pstrAddr, const unsigned short usPort);
private:
	inline void				AddAvailableConnection(CTcpConnection *pConnection)
	{
		if (m_uFreeConnIndex)
		{
			m_pFreeConn[--m_uFreeConnIndex]	= pConnection;
		}
	}

	int						SetNoBlocking(CTcpConnection *pTcpConnection);
	void					RemoveConnection(CTcpConnection *pTcpConnection);

	void					ProcessConnectedConnection();
	void					ProcessWaitConnectConnection();
	void					ProcessWaitCloseConnection();
	bool					IsConnectSuccess(CTcpConnection *pNetLink);
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
