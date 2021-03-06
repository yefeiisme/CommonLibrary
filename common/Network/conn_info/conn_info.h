#ifndef __NET_LINK_H_
#define __NET_LINK_H_

#include "INetwork.h"
#include "../NetworkHead.h"
#define MAX_IP_LEN	128

enum E_NET_LINK_STATE
{
	NET_LINK_STATE_DISCONNNECT = -1,
	NET_LINK_STATE_CONNECT,
	NET_LINK_STATE_WAI_CONNECT,
};

class CTcpConnection : public ITcpConnection
{
public:
	SOCKET						m_nSock;
	unsigned int				m_uConnID;
private:
	E_NET_LINK_STATE			m_eStatus;

	/**********************发送缓冲区**********************/
	char						*m_pSendBuf;
	char						*m_pTempSendBuf;
	char						*m_pFlush;
	char						*m_pSend;
	unsigned int				m_uSendBufLen;
	unsigned int				m_uTempSendBufLen;
	/**********************发送缓冲区**********************/
	
	/**********************接收缓冲区**********************/
	char						*m_pRecvBuf;
	char						*m_pTempRecvBuf;
	char						*m_pPack;
	char						*m_pRecv;
	char						*m_pUnreleased;
	unsigned int				m_uRecvBufLen;
	unsigned int				m_uTempRecvBufLen;
	/**********************接收缓冲区**********************/

	bool						m_bTcpConnected;		// 网络连接是否连接状态
	bool						m_bLogicConnected;		// 外部逻辑是否连接状态
	bool						m_bConnectSuccess;		// 异步连接是否完成
public:
	CTcpConnection();
	~CTcpConnection();

	inline bool					IsSocketConnected()
	{
		return m_bTcpConnected;
	}

	inline bool					IsLogicConnected()
	{
		return m_bLogicConnected;
	}

	inline bool					IsConnectSuccess()
	{
		return m_bConnectSuccess;
	}

	inline bool					IsConnect()
	{
		return NET_LINK_STATE_CONNECT == m_eStatus;
	}

	inline bool					IsDisconnected()
	{
		return NET_LINK_STATE_DISCONNNECT == m_eStatus;
	}

	inline bool					IsWaitConnect()
	{
		return NET_LINK_STATE_WAI_CONNECT == m_eStatus;
	}

	bool						Initialize(unsigned int uRecvBufferLen, unsigned int uSendBufferLen, unsigned int uTempRecvBufLen, unsigned int uTempSendBufLen);
	inline void					ReInit(const int nSocket)
	{
		m_pUnreleased	= m_pRecv = m_pPack = m_pRecvBuf;
		m_pFlush		= m_pSend = m_pSendBuf;
		m_nSock			= nSocket;
	}

	const char					*GetIP();

	int							RecvData();
	int							SendData();
	bool						PutPack(const void *pPack, unsigned int uPackLen);
	const void					*GetPack(unsigned int &uPackLen);

	inline void					ShutDown()
	{
		m_bLogicConnected	= false;
	}

	void						Disconnect();

	inline void					TcpConnected()
	{
		m_bTcpConnected	= true;
	}

	inline void					ConnectSuccess()
	{
		m_bConnectSuccess	= true;
	}

	inline void					LogicConnected()
	{
		m_bLogicConnected	= true;
	}

	inline void					AllConnected()
	{
		m_bTcpConnected	= true;
		m_bLogicConnected	= true;
	}
};

#endif
