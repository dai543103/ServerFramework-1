#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "ServerListener.hpp"

CServerListener::CServerListener()
{
	m_uiIP = 0;
	m_ushPort = 0;
	m_uiSendBufferSize = 0;
	m_iState = LotusListener_UnListen;
}

CServerListener::~CServerListener()
{

}

int CServerListener::Init(unsigned int uiIP, unsigned short ushPort, unsigned int uiSendBufferSize)
{
	m_uiIP = uiIP;
	m_ushPort = ushPort;
	m_uiSendBufferSize = uiSendBufferSize;
	m_iState = LotusListener_UnListen;

	return 0;
}


int CServerListener::GetFD()
{
	return m_iListeningSocket;
}

int CServerListener::CreateTCPSocket()
{
	m_iListeningSocket = socket(AF_INET, SOCK_STREAM, 0);
	if(m_iListeningSocket < 0)
	{
		return -1;
	}
	printf("ListeningSocketFD:%d\n", m_iListeningSocket);
	return 0;
}

int CServerListener::Bind(unsigned int uiIP, unsigned short ushPort)
{
	struct sockaddr_in stSocketAddress;
	memset(&stSocketAddress, 0, sizeof(stSocketAddress));
	stSocketAddress.sin_family = AF_INET;
	stSocketAddress.sin_port = htons(ushPort);
	stSocketAddress.sin_addr.s_addr = uiIP;

	int iRet = bind(m_iListeningSocket, (struct sockaddr *)&stSocketAddress, sizeof(stSocketAddress));
	if(iRet < 0)
	{
		printf("bind %d failed, %s\n", ushPort, strerror(errno));
		return -1;
	}
	printf("bind %d OK\n", ushPort);

	return 0;
}

int CServerListener::Listen()
{
	int iRet = listen(m_iListeningSocket, 1024);
	if(iRet < 0)
	{
		printf("listen %d connection error!\n", 1024);
		return -1;
	}

	m_iState = LotusListener_Listen;

	return 0;
}

int CServerListener::SetSendBufferSize(int iBufferSize)
{
	//ÉèÖÃ·¢ËÍ»º³å³ß´ç
	int iOptLength = sizeof(socklen_t);
	int iOptValue = iBufferSize;
	if(setsockopt(m_iListeningSocket, SOL_SOCKET, SO_SNDBUF, (const void *)&iOptValue, iOptLength))
	{
		printf("Set send buffer size to %d failed!\n", iOptValue);
		return -1;
	}
	if(!getsockopt(m_iListeningSocket, SOL_SOCKET, SO_SNDBUF, (void *)&iOptValue, (socklen_t *)&iOptLength))
	{
		printf("Set Send buf of socket is %d.\n", iOptValue);
	}

	return 0;
}

int CServerListener::SetForEpoll()
{
	SetReuseAddress();
	SetKeepAlive();
	SetLingerOff();

	return 0;
}

int CServerListener::SetReuseAddress()
{
	int iFlag = 1;
	setsockopt(m_iListeningSocket, SOL_SOCKET, SO_REUSEADDR, &iFlag, sizeof(iFlag));
	return 0;
}

int CServerListener::SetKeepAlive()
{
	int iFlag = 1;
	setsockopt(m_iListeningSocket, SOL_SOCKET, SO_KEEPALIVE, &iFlag, sizeof(iFlag));
	return 0;
}

int CServerListener::SetLingerOff()
{
	struct linger stLinger;
	stLinger.l_onoff = 0;
	stLinger.l_linger = 0;
	setsockopt(m_iListeningSocket, SOL_SOCKET, SO_LINGER, &stLinger, sizeof(stLinger));
	return 0;
}
