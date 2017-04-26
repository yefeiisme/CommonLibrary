#ifndef _I_SERVER_H_
#define _I_SERVER_H_

#include <list>
#include "../NetworkHead.h"

using namespace std;

class CServerNetwork : public IServerNetwork
{
private:
	CALLBACK_SERVER_EVENT	m_pfnConnectCallBack;
	void					*m_pFunParam;

	CTcpConnection			*m_pListenLink;

	CTcpConnection			*m_pTcpConnection;		// ���е���������
	CTcpConnection			**m_pFreeConn;			// ��ǰ�������״̬��CNetLink��������

	unsigned int			m_uMaxConnCount;
	unsigned int			m_uFreeConnIndex;		// m_pFreeLink������������list��iterator�÷�

	unsigned int			m_uClientRecvBuffSize;
	unsigned int			m_uClientSendBuffSize;

	unsigned int			m_uThreadFrame;

#ifdef __linux
	int						m_nepfd;
#elif defined(WIN32) || defined(WIN64)
	fd_set					m_ReadSet;
	fd_set					m_ErrorSet;
#elif defined(__APPLE__)
#endif

	list<CTcpConnection*>	m_listActiveConn;
	list<CTcpConnection*>	m_listCloseWaitConn;

	bool					m_bRunning;
	bool					m_bThread;
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
		Sleep(m_uThreadFrame);
#else
		struct timeval sleeptime;
		sleeptime.tv_sec	= 0;
		sleeptime.tv_usec	= m_uThreadFrame * 1000;
		select(0, 0, 0, 0, &sleeptime);
#endif
	}
public:
	CServerNetwork();
	~CServerNetwork();

	bool					Initialize(
										const unsigned short usPort,
										void *lpParam,
										CALLBACK_SERVER_EVENT pfnConnectCallBack,
										const unsigned int uConnectionNum,
										const unsigned int uSendBufferLen,
										const unsigned int uRecvBufferLen,
										const unsigned int uTempSendBufferLen,
										const unsigned int uTempRecvBufferLen,
										const bool bThread,
										const unsigned int uSleepFrame
										);
	void					Stop();
	void					Release();
	void					CloseAcceptor();
	void					DoNetworkAction();
};

#endif
