#pragma once

enum 
{
	LotusListener_UnListen = 1,
	LotusListener_Listen = 2,
};

class CServerListener
{
public:
	CServerListener();
	~CServerListener();
	
	int Init(unsigned int uiIP, unsigned short ushPort, unsigned int uiSendBufferSize);

	int GetFD();

	int CreateTCPSocket();
	int Bind(unsigned int uiIP, unsigned short ushPort);
	int Listen();

	int SetSendBufferSize(int iBufferSize);

	int SetForEpoll();

private:

	int SetReuseAddress();
	int SetKeepAlive();
	int SetLingerOff();

public:
	unsigned int m_uiIP;
	unsigned short m_ushPort;
	unsigned int m_uiSendBufferSize;
	int m_iState; 

private:
	int m_iListeningSocket;
};
