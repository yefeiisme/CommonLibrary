#include "../conn_info/conn_info.h"
#include "INetwork.h"
#include "server_network_select.h"
#include "IFileLog.h"
#include <thread>

CServerNetwork::CServerNetwork()
{
	m_pfnConnectCallBack	= nullptr;
	m_pFunParam				= nullptr;

	m_pListenLink			= nullptr;

	m_pTcpConnection		= nullptr;
	m_pFreeConn				= nullptr;
	m_uMaxConnCount			= 0;
	m_uFreeConnIndex		= 0;

	m_uSleepTime			= 0;

	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);

	m_listActiveConn.clear();
	m_listCloseWaitConn.clear();

	m_bRunning				= false;
	m_bExited				= false;
}

CServerNetwork::~CServerNetwork()
{
	if (m_pListenLink->IsSocketConnected())
	{
		DisconnectConnection(m_pListenLink);
	}

	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].IsSocketConnected())
		{
			DisconnectConnection(&m_pTcpConnection[nIndex]);
		}
	}

	SAFE_DELETE(m_pListenLink);
	SAFE_DELETE_ARR(m_pTcpConnection);

#if defined(WIN32) || (WIN64)
	WSACleanup();
#endif
}

int CServerNetwork::SetNoBlocking(CTcpConnection *pTcpConnection)
{
	if (nullptr == pTcpConnection)
		return -1;

#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	return ioctlsocket(pTcpConnection->GetSock(), FIONBIO, &ulNonBlock);
#elif defined(__linux)
	int nFlags = fcntl(pTcpConnection->GetSock(), F_GETFL, 0);
	if (nFlags < 0 || fcntl(pTcpConnection->GetSock(), F_SETFL, nFlags | O_NONBLOCK | O_ASYNC) < 0)
		return -1;

	return 0;
#elif defined(__APPLE__)
#endif
}

void CServerNetwork::AcceptClient(const SOCKET nNewSocket)
{
	CTcpConnection	*pNewLink	= GetNewConnection();
	if (nullptr == pNewLink)
	{
		closesocket(nNewSocket);
		return;
	}

	pNewLink->ReInit(nNewSocket);

	pNewLink->Connected();

	if (-1 == SetNoBlocking(pNewLink))
	{
		RemoveConnection(pNewLink);

#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("ioctlsocket() failed with error %d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("ERROR on rtsig the sock; errno=%d\n", errno);
#elif defined(__APPLE__)
#endif

		return;
	}

	m_listActiveConn.push_back(pNewLink);

	m_pfnConnectCallBack(m_pFunParam, pNewLink, pNewLink->GetConnID());
}

void CServerNetwork::DisconnectConnection(CTcpConnection *pTcpConnection)
{
	pTcpConnection->Disconnect();
}

void CServerNetwork::RemoveConnection(CTcpConnection *pTcpConnection)
{
	DisconnectConnection(pTcpConnection);

	AddAvailableConnection(pTcpConnection);
}

void CServerNetwork::CloseConnection(CTcpConnection *pTcpConnection)
{
	DisconnectConnection(pTcpConnection);

	m_listCloseWaitConn.push_back(pTcpConnection);
}

void CServerNetwork::ReadAction()
{
	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_ErrorSet);

	FD_SET(m_pListenLink->GetSock(), &m_ReadSet);
	timeval	timeout	={ 0, 0 };

	if (select(1024, &m_ReadSet, nullptr, nullptr, &timeout) > 0)
	{
		if (FD_ISSET(m_pListenLink->GetSock(), &m_ReadSet))
		{
			sockaddr_in	client_addr;
			socklen_t	length	= sizeof(client_addr);
			SOCKET	nNewSocket	= accept(m_pListenLink->GetSock(), (sockaddr*)&client_addr, &length);
			if (INVALID_SOCKET != nNewSocket)
			{
				AcceptClient(nNewSocket);
			}
		}
	}

	CTcpConnection	*pTcpConnection	= nullptr;

	for (auto Iter = m_listActiveConn.begin(); Iter != m_listActiveConn.end();)
	{
		pTcpConnection	= *Iter;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_ErrorSet);

		FD_SET(pTcpConnection->GetSock(), &m_ReadSet);
		FD_SET(pTcpConnection->GetSock(), &m_ErrorSet);

		if (select(1024, &m_ReadSet, nullptr, &m_ErrorSet, &timeout) <= 0)
		{
			++Iter;
			continue;
		}

		if (FD_ISSET(pTcpConnection->GetSock(), &m_ReadSet))
		{
			if (pTcpConnection->RecvData() == -1)
			{
				CloseConnection(pTcpConnection);
				Iter	= m_listActiveConn.erase(Iter);
			}
		}
		else if (FD_ISSET(pTcpConnection->GetSock(), &m_ErrorSet))
		{
			CloseConnection(pTcpConnection);
			Iter	= m_listActiveConn.erase(Iter);
		}
		else
		{
			++Iter;
		}
	}
}

void CServerNetwork::WriteAction()
{
	CTcpConnection	*pTcpConnection	= nullptr;
	for (auto Iter = m_listActiveConn.begin(); Iter != m_listActiveConn.end();)
	{
		pTcpConnection	= *Iter;
		if (!pTcpConnection->IsLogicConnected())
		{
			// 逻辑层已经退出了，这里只需要将TcpConnect从Active中移除OK了
			RemoveConnection(pTcpConnection);
			Iter	= m_listActiveConn.erase(Iter);
			continue;
		}

		if (pTcpConnection->SendData() == -1)
		{
			// 网络层异常，关闭网络相关操作，放入等待队列中，等待逻辑层退出
			CloseConnection(pTcpConnection);
			Iter	= m_listActiveConn.erase(Iter);
			continue;
		}

		++Iter;
	}
}

void CServerNetwork::CloseAction()
{
	CTcpConnection	*pTcpConnection	= nullptr;
	for (auto Iter = m_listCloseWaitConn.begin(); Iter != m_listCloseWaitConn.end();)
	{
		pTcpConnection	= *Iter;
		if (pTcpConnection->IsLogicConnected())
		{
			++Iter;
			continue;
		}

		AddAvailableConnection(pTcpConnection);

		Iter	= m_listCloseWaitConn.erase(Iter);
	}
}

void CServerNetwork::ThreadFunc()
{
	while (m_bRunning)
	{
		ReadAction();
		WriteAction();
		CloseAction();

		yield();
	}

	// 关闭accept
	DisconnectConnection(m_pListenLink);

	// 关闭已连接上的Connection
	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].IsSocketConnected())
		{
			DisconnectConnection(&m_pTcpConnection[nIndex]);
		}
	}

	m_bExited	= true;
}

bool CServerNetwork::Initialize(
	const unsigned short usPort,
	void *lpParam,
	pfnConnectEvent pfnConnectCallBack,
	const unsigned int uConnectionNum,
	const unsigned int uSendBufferLen,
	const unsigned int uRecvBufferLen,
	const unsigned int uTempSendBufferLen,
	const unsigned int uTempRecvBufferLen,
	const unsigned int uSleepTime
	)
{
#if defined(WIN32) || defined(WIN64)
	WORD wVersionRequested;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	if (WSAStartup(wVersionRequested, &wsaData) != 0)
	{
		return false;
	}
#endif

	m_uMaxConnCount			= uConnectionNum;
	m_pfnConnectCallBack	= pfnConnectCallBack;
	m_pFunParam				= lpParam;

	if (0 == m_uMaxConnCount)
		return false;

	if (0 == uRecvBufferLen)
		return false;

	if (0 == uSendBufferLen)
		return false;

	if (nullptr == lpParam)
		return false;

	if (nullptr == pfnConnectCallBack)
		return false;

	if (nullptr == lpParam)
		return false;

	m_pListenLink	= new CTcpConnection;
	if (nullptr == m_pListenLink)
		return false;

	m_pTcpConnection	= new CTcpConnection[m_uMaxConnCount];
	if (nullptr == m_pTcpConnection)
		return false;

	m_pFreeConn	= new CTcpConnection*[m_uMaxConnCount];
	if (!m_pFreeConn)
		return false;

	for (unsigned int uIndex = 0; uIndex < m_uMaxConnCount; ++uIndex)
	{
		if (!m_pTcpConnection[uIndex].Initialize(uIndex, uRecvBufferLen, uSendBufferLen, uTempSendBufferLen, uTempRecvBufferLen))
			return false;

		m_pFreeConn[uIndex]	= &m_pTcpConnection[uIndex];
	}

	struct sockaddr_in tagAddr;
	memset(&tagAddr, 0, sizeof(tagAddr));
	tagAddr.sin_family		= AF_INET;
	tagAddr.sin_port		= htons(usPort);
	tagAddr.sin_addr.s_addr	= INADDR_ANY;

	// start listen
	SOCKET	nSock	= socket(AF_INET, SOCK_STREAM, 0);
	if (nSock < 0)
	{
		g_pFileLog->WriteLog("create listen socket failed!\n");
		return false;
	}

	m_pListenLink->SetSock(nSock);

	int nOnFlag = 1;
#if defined(WIN32) || defined(WIN64)
	setsockopt(m_pListenLink->GetSock(), SOL_SOCKET, SO_REUSEADDR, (const char*)&nOnFlag, sizeof(nOnFlag));
#elif defined(__linux)
	setsockopt(m_pListenLink->GetSock(), SOL_SOCKET, SO_REUSEADDR, (const void*)&nOnFlag, sizeof(nOnFlag));
#elif defined(__APPLE__)
#endif

	if (::bind(m_pListenLink->GetSock(), (const sockaddr *)&tagAddr, sizeof(tagAddr)))
	{
		g_pFileLog->WriteLog("bind listen socket failed!\n");
		return false;
	}

	if (listen(m_pListenLink->GetSock(), 4096))
	{
		g_pFileLog->WriteLog("listen socket failed!\n");
		return false;
	}

	if (-1 == SetNoBlocking(m_pListenLink))
	{
#if defined(WIN32) || defined(WIN64)
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", WSAGetLastError());
#elif defined(__linux)
		g_pFileLog->WriteLog("Set listen socket async failed! errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
		return false;
	}

	m_uSleepTime	= uSleepTime;
	m_bExited		= false;
	m_bRunning		= true;

	std::thread	threadNetwork(&CServerNetwork::ThreadFunc, this);
	threadNetwork.detach();

	return true;
}

void CServerNetwork::Release()
{
	delete this;
}

IServerNetwork *CreateServerNetwork(
	unsigned short usPort,
	void *lpParam,
	pfnConnectEvent pfnConnectCallBack,
	unsigned int uConnectionNum,
	unsigned int uSendBufferLen,
	unsigned int uRecvBufferLen,
	unsigned int uTempSendBufferLen,
	unsigned int uTempRecvBufferLen,
	const unsigned int uSleepTime
)
{
	CServerNetwork	*pServer = new CServerNetwork();
	if (nullptr == pServer)
		return nullptr;

	if (!pServer->Initialize(usPort, lpParam, pfnConnectCallBack, uConnectionNum, uSendBufferLen, uRecvBufferLen, uTempSendBufferLen, uTempRecvBufferLen, uSleepTime))
	{
		pServer->Release();
		return nullptr;
	}

	return pServer;
}
