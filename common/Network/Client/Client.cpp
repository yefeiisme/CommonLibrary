#include "../NetworkHead.h"
#include "INetwork.h"
#include "../conn_info/conn_info.h"
#include "Client.h"
#include "IFileLog.h"
#include <thread>

#ifdef _MSC_VER
#define snprintf _snprintf
#endif

CClientNetwork::CClientNetwork()
{
	m_pfnConnectCallBack	= nullptr;
	m_pFunParam				= nullptr;

	m_pTcpConnection		= nullptr;
	m_pFreeConn				= nullptr;

	m_pConnectBuffer		= nullptr;

	m_uMaxConnCount			= 0;
	m_uFreeConnIndex		= 0;

	m_uSleepTime			= 0;

	m_listActiveConn.clear();
	m_listWaitConnectedConn.clear();
	m_listCloseWaitConn.clear();

	FD_ZERO(&m_ReadSet);
	FD_ZERO(&m_WriteSet);
	FD_ZERO(&m_ErrorSet);

	m_bRunning				= false;
	m_bExited				= false;
}

CClientNetwork::~CClientNetwork()
{
	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].IsConnect())
		{
			m_pTcpConnection[nIndex].Disconnect();
		}
	}

	SAFE_DELETE_ARR(m_pTcpConnection);
	m_uMaxConnCount	= 0;
	SAFE_DELETE_ARR(m_pFreeConn);
	m_uFreeConnIndex	= 0;

	if (m_pConnectBuffer)
	{
		m_pConnectBuffer->Release();
		m_pConnectBuffer	= nullptr;
	}

#if defined(WIN32) || defined(WIN64)
	WSACleanup();
#endif
}

bool CClientNetwork::Initialize(
	const unsigned int uClientCount,
	const unsigned int uSendBuffLen,
	const unsigned int uRecvBuffLen,
	const unsigned int uTempSendBuffLen,
	const unsigned int uTempRecvBuffLen,
	CALLBACK_CLIENT_EVENT pfnConnectCallBack,
	void *lpParm,
	const unsigned int uSleepTime
	)
{
	if (nullptr == pfnConnectCallBack)
		return false;

	if (nullptr == lpParm)
		return false;

	m_uMaxConnCount	= uClientCount;

#if defined(WIN32) || defined(WIN64)
	WSADATA	wsaData;
	WORD	wVersionRequested	= MAKEWORD(2, 2);
	int		nError				= WSAStartup(wVersionRequested, &wsaData);

	if (nError != 0)
		return false;
#endif

	m_pConnectBuffer	= CreateRingBuffer(uClientCount*sizeof(SConnectRequest), sizeof(SConnectRequest));
	if (nullptr == m_pConnectBuffer)
		return false;

	m_pTcpConnection	= new CTcpConnection[m_uMaxConnCount];
	if (nullptr == m_pTcpConnection)
		return false;

	m_pFreeConn	= new CTcpConnection*[m_uMaxConnCount];
	if (nullptr == m_pFreeConn)
		return false;

	for (unsigned int uIndex = 0; uIndex < m_uMaxConnCount; ++uIndex)
	{
		if (!m_pTcpConnection[uIndex].Initialize(uRecvBuffLen, uSendBuffLen, uTempRecvBuffLen, uTempSendBuffLen))
			return false;

		m_pTcpConnection[uIndex].SetConnID(uIndex);

		m_pFreeConn[uIndex]	= &m_pTcpConnection[uIndex];
	}

	m_pfnConnectCallBack	= pfnConnectCallBack;
	m_pFunParam				= lpParm;

	m_uSleepTime			= uSleepTime;
	m_bExited				= false;
	m_bRunning				= true;

	std::thread	threadNetwork(&CClientNetwork::ThreadFunc, this);
	threadNetwork.detach();

	return true;
}

void CClientNetwork::Release()
{
	delete this;
}

bool CClientNetwork::ConnectTo(char *pstrAddr, const unsigned short usPort, void *pTarget)
{
	SConnectRequest	tagRequest;
	memset(&tagRequest, 0, sizeof(tagRequest));
	tagRequest.pTarget	= pTarget;
	tagRequest.usPort	= usPort;
	strncpy(tagRequest.strAddr, pstrAddr, sizeof(tagRequest.strAddr));
	tagRequest.strAddr[sizeof(tagRequest.strAddr)-1]	= '\0';

	return m_pConnectBuffer->SndPack(&tagRequest, sizeof(tagRequest));
}

void CClientNetwork::TryConnect(const void *pPack)
{
	SConnectRequest	*pRequest	= (SConnectRequest*)pPack;

	if (m_uFreeConnIndex >= m_uMaxConnCount)
	{
		m_pfnConnectCallBack(m_pFunParam, nullptr, pRequest->pTarget);
		return;
	}

	int	nNewSock = socket(AF_INET, SOCK_STREAM, 0);
	if (nNewSock < 0)
	{
		g_pFileLog->WriteLog("socket failed,sock = %d\n", nNewSock);
		m_pfnConnectCallBack(m_pFunParam, nullptr, pRequest->pTarget);
		return;
	}

#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	if (ioctlsocket(nNewSock, FIONBIO, &ulNonBlock) == SOCKET_ERROR)
#elif defined(__linux)
	int nFlags = fcntl(nNewSock, F_GETFL, 0);
	if (nFlags < 0 || fcntl(nNewSock, F_SETFL, nFlags | O_NONBLOCK | O_ASYNC) < 0)
#elif defined(__APPLE__)
#endif
	{
		closesocket(nNewSock);
		m_pfnConnectCallBack(m_pFunParam, nullptr, pRequest->pTarget);
		return;
	}

	sockaddr_in	tagAddrIn;

	memset(&tagAddrIn, 0, sizeof(tagAddrIn));
	tagAddrIn.sin_family		= AF_INET;
	tagAddrIn.sin_port			= htons(pRequest->usPort);
	tagAddrIn.sin_addr.s_addr	= inet_addr(pRequest->strAddr);

	int nRet = connect(nNewSock, (sockaddr*)&tagAddrIn, sizeof(tagAddrIn));
	if (0 == nRet)
	{
		CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

		++m_uFreeConnIndex;

		pNewLink->ReInit(nNewSock);

		pNewLink->Connected();

		m_listActiveConn.push_back(pNewLink);

		m_pfnConnectCallBack(m_pFunParam, pNewLink, pRequest->pTarget);
	}
#if defined(WIN32) || defined(WIN64)
	else if (WSAGetLastError() == WSAEWOULDBLOCK)
#elif defined(__linux)
	else if (EINPROGRESS == errno)
#elif defined(__APPLE__)
#endif
	{
		CTcpConnection	*pNewLink = m_pFreeConn[m_uFreeConnIndex];

		++m_uFreeConnIndex;

		pNewLink->ReInit(nNewSock);

		pNewLink->Connected();

		pNewLink->SetConnectTarget(pRequest->pTarget);

		m_listWaitConnectedConn.push_back(pNewLink);
	}
	else
	{
		closesocket(nNewSock);

		m_pfnConnectCallBack(m_pFunParam, nullptr, pRequest->pTarget);
	}
}

int CClientNetwork::SetNoBlocking(CTcpConnection *pTcpConnection)
{
#if defined(WIN32) || defined(WIN64)
	unsigned long ulNonBlock = 1;
	return (ioctlsocket(pTcpConnection->GetSock(), FIONBIO, &ulNonBlock) == SOCKET_ERROR);
#elif defined(__linux)
	int nFlags = fcntl(pTcpConnection->GetSock(), F_GETFL, 0);
	if (nFlags < 0 || fcntl(pTcpConnection->GetSock(), F_SETFL, nFlags | O_NONBLOCK | O_ASYNC) < 0)
		return -1;

	return 0;
#elif defined(__APPLE__)
#endif
}

void CClientNetwork::RemoveConnection(CTcpConnection *pTcpConnection)
{
	pTcpConnection->Disconnect();
}

void CClientNetwork::ProcessConnectRequest()
{
	const void		*pPack	= nullptr;
	unsigned int	uPackLen	= 0;

	while (nullptr != (pPack = m_pConnectBuffer->RcvPack(uPackLen)))
	{
		TryConnect(pPack);
	}
}

void CClientNetwork::ProcessConnectedConnection()
{
	timeval			timeout			= {0, 0};
	CTcpConnection	*pTcpConnection	= nullptr;

	for (list<CTcpConnection*>::iterator Iter = m_listActiveConn.begin(); Iter != m_listActiveConn.end();)
	{
		pTcpConnection	= *Iter;

		FD_ZERO(&m_ReadSet);
		FD_ZERO(&m_WriteSet);
		FD_ZERO(&m_ErrorSet);

		FD_SET(pTcpConnection->GetSock(), &m_ReadSet);
		FD_SET(pTcpConnection->GetSock(), &m_WriteSet);
		FD_SET(pTcpConnection->GetSock(), &m_ErrorSet);

		if (select(1024, &m_ReadSet, &m_WriteSet, &m_ErrorSet, &timeout) <= 0)
		{
			++Iter;
			return;
		}

		if (FD_ISSET(pTcpConnection->GetSock(), &m_ErrorSet))
		{
			RemoveConnection(pTcpConnection);
			m_listCloseWaitConn.push_back(pTcpConnection);
			Iter	= m_listActiveConn.erase(Iter);
			continue;
		}

		if (FD_ISSET(pTcpConnection->GetSock(), &m_ReadSet))
		{
			if (pTcpConnection->RecvData() == -1)
			{
				RemoveConnection(pTcpConnection);
				m_listCloseWaitConn.push_back(pTcpConnection);
				Iter	= m_listActiveConn.erase(Iter);
				continue;
			}
		}

		if (FD_ISSET(pTcpConnection->GetSock(), &m_WriteSet))
		{
			if (pTcpConnection->SendData() == -1)
			{
				RemoveConnection(pTcpConnection);
				m_listCloseWaitConn.push_back(pTcpConnection);
				Iter	= m_listActiveConn.erase(Iter);
				continue;
			}
		}

		++Iter;
	}
}

void CClientNetwork::ProcessWaitConnectConnection()
{
	timeval			timeout			= {0, 0};
	CTcpConnection	*pTcpConnection	= nullptr;

	for (list<CTcpConnection*>::iterator Iter = m_listWaitConnectedConn.begin(); Iter != m_listWaitConnectedConn.end(); ++Iter)
	{
		pTcpConnection	= *Iter;

		FD_ZERO(&m_WriteSet);

		FD_SET(pTcpConnection->GetSock(), &m_WriteSet);

		if (select(0, &m_ReadSet, &m_WriteSet, nullptr, &timeout) <= 0)
		{
			++Iter;
			continue;
		}

		if (!FD_ISSET(pTcpConnection->GetSock(), &m_WriteSet))
		{
			++Iter;
			continue;
		}

		int nError = 0;
		socklen_t len = sizeof(nError);
#if defined(WIN32) || defined(WIN64)
		if (getsockopt(pTcpConnection->GetSock(), SOL_SOCKET, SO_ERROR, (char*)&nError, &len) < 0)
#elif defined(__linux)
		if (getsockopt(pTcpConnection->GetSock(), SOL_SOCKET, SO_ERROR, &nError, &len) < 0)
#elif defined(__APPLE__)
#endif
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("getsockopt Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			m_pfnConnectCallBack(m_pFunParam, nullptr, pTcpConnection->GetConnectTarget());

			continue;
		}

		if (nError != 0)
		{
#if defined(WIN32) || defined(WIN64)
			g_pFileLog->WriteLog("Connnect Failed errno=%d\n", WSAGetLastError());
#elif defined(__linux)
			g_pFileLog->WriteLog("Connnect Failed errno=%d\n", errno);
#elif defined(__APPLE__)
#endif
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			m_pfnConnectCallBack(m_pFunParam, nullptr, pTcpConnection->GetConnectTarget());

			continue;
		}

		if (-1 == SetNoBlocking(pTcpConnection))
		{
			RemoveConnection(pTcpConnection);

			AddAvailableConnection(pTcpConnection);

			Iter	= m_listWaitConnectedConn.erase(Iter);

			m_pfnConnectCallBack(m_pFunParam, nullptr, pTcpConnection->GetConnectTarget());

			continue;
		}

		m_listActiveConn.push_back(pTcpConnection);

		Iter	= m_listWaitConnectedConn.erase(Iter);

		m_pfnConnectCallBack(m_pFunParam, pTcpConnection, pTcpConnection->GetConnectTarget());
	}
}

void CClientNetwork::ProcessWaitCloseConnection()
{
	CTcpConnection	*pTcpConnection	= nullptr;

	for (list<CTcpConnection*>::iterator Iter = m_listCloseWaitConn.begin(); Iter != m_listCloseWaitConn.end();)
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

void CClientNetwork::ThreadFunc()
{
	while (m_bRunning)
	{
		ProcessConnectedConnection();

		ProcessWaitConnectConnection();

		ProcessWaitCloseConnection();

		yield();
	}

	// 关闭已连接上的Connection
	for (int nIndex = 0; nIndex < m_uMaxConnCount; ++nIndex)
	{
		if (m_pTcpConnection[nIndex].IsSocketConnected())
		{
			m_pTcpConnection[nIndex].Disconnect();
		}
	}

	m_bExited	= true;
}

IClientNetwork *CreateClientNetwork(
	unsigned int uLinkCount,
	unsigned int uMaxSendBuff,
	unsigned int uMaxReceiveBuff,
	unsigned int uMaxTempSendBuff,
	unsigned int uMaxTempReceiveBuff,
	CALLBACK_CLIENT_EVENT pfnConnectCallBack,
	void *lpParm,
	const unsigned int uSleepTime
	)
{
	CClientNetwork	*pClient = new CClientNetwork();
	if (nullptr == pClient)
		return nullptr;

	if (!pClient->Initialize(uLinkCount, uMaxSendBuff, uMaxReceiveBuff, uMaxTempSendBuff, uMaxTempReceiveBuff, pfnConnectCallBack, lpParm, uSleepTime))
	{
		pClient->Release();
		return nullptr;
	}

	return pClient;
}
