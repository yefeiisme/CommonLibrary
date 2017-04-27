#ifndef __I_NETWORK_H_
#define __I_NETWORK_H_

class ITcpConnection
{
public:
	virtual bool		IsConnectSuccess() = 0;											// ֻ�����첽Connectʱ��Clientȥ�ж��Ƿ��Ѿ�������Server��������������õ��ô˺���
	virtual bool		IsConnect() = 0;												// �������߼����ж��Ƿ�������״̬
	virtual const void	*GetPack(unsigned int &uPackLen) = 0;							// ��ȡ���յ����������ݰ�
	virtual bool		PutPack(const void *pPack, unsigned int uPackLen) = 0;			// �����������ݰ�
	virtual void		ShutDown() = 0;													// �Ͽ�������1.�����ⲿ���߼��㣩��������ĶϿ����ӣ�2.�����ⲿ���߼��㣩�յ�����Ͽ�����Ϣ���������Ӧ���������֪ͨ�������������Ͽ���
	virtual const char	*GetIP() = 0;
};

class IServerNetwork
{
public:
	virtual void		Stop() = 0;			// �˳�ʱ���ã����������߳��˳���
	virtual bool		IsExited() = 0;		// ����ȷ�������߳��Ƿ��Ѿ��������˳�
	virtual void		Release() = 0;		// �ͷ��ڴ�ռ�
};

class IClientNetwork
{
public:
	virtual void		Stop() = 0;																	// �˳�ʱ���ã����������߳��˳���
	virtual bool		IsExited() = 0;																// ����ȷ�������߳��Ƿ��Ѿ��������˳�
	virtual void		Release() = 0;																// �ͷ��ڴ�ռ�
	virtual bool		ConnectTo(char *pstrAddr, const unsigned short usPort, void *pTarget) = 0;	// ���ӷ�����
};

typedef void(*CALLBACK_SERVER_EVENT)(void *lpParam, ITcpConnection *pTcpConnection);
typedef void(*CALLBACK_CLIENT_EVENT)(void *lpParam, ITcpConnection *pTcpConnection, const void *pTarget);

IServerNetwork *CreateServerNetwork(
	const unsigned short usPort,				// �˿ں�
	void *lpParam,								// �ص������Ĳ���
	CALLBACK_SERVER_EVENT pfnConnectCallBack,	// ���ӳɹ���Ļص�����
	const unsigned int uConnectionNum,			// ���������
	const unsigned int uSendBufferLen,			// ÿ�����ӷ��ͻ������Ĵ�С
	const unsigned int uRecvBufferLen,			// ÿ�����ӽ��ջ������Ĵ�С
	const unsigned int uTempSendBufferLen,		// ����Ͱ��Ĵ�С
	const unsigned int uTempRecvBufferLen,		// �����հ��Ĵ�С
	const unsigned int uSleepTime				// �̵߳�Sleepʱ��
	);
IClientNetwork *CreateClientNetwork(
	const unsigned int uConnectionNum,			// ���������
	const unsigned int uSendBufferLen,			// ÿ�����ӷ��ͻ������Ĵ�С
	const unsigned int uRecvBufferLen,			// ÿ�����ӽ��ջ������Ĵ�С
	const unsigned int uTempSendBufferLen,		// ����Ͱ��Ĵ�С
	const unsigned int uTempRecvBufferLen,		// �����հ��Ĵ�С
	CALLBACK_CLIENT_EVENT pfnConnectCallBack,	// ���ӳɹ���Ļص�����
	void *lpParm,								// �ص������Ĳ���
	const unsigned int uSleepTime				// �̵߳�Sleepʱ��
	);

#endif
