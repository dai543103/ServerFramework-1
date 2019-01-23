#pragma once

//TCP连接控制信息
#define PLAYERNOTIFYTCPCLOSESOCKET  1

#define	APP_CONFIG_FILE		"../conf/GameServer.tcm"

static const int APPCMD_STOP_SERVICE = 1;
static const int APPCMD_RELOAD_CONFIG = 2;

#ifdef _DEBUG_
static const unsigned int FD_SIZE = 10000; //方便测试
#else
static const unsigned int FD_SIZE = 10000;
#endif

static const unsigned int EXTERNALCLIENT_RECVBUFFER_SIZE = 4096;
static const unsigned int MAX_LISTENER_NUMBER = 16;
static const unsigned int MAX_FILENAME_LENGTH = 256;
static const unsigned int DEFAULT_SENDBUFFER_SIZE = 2048;
static const unsigned int DEFAULT_RECVBUFFER_SIZE = 2048;
static const unsigned int DEFAULT_INPUTPACKET_LENGTHRESTRCIT = 2048;

typedef enum enLotusSocketFlag
{
    ELSF_ListeningForExternalClient = (unsigned int)0x00000001,
    ELSF_ConnectedByExternalClient	= (unsigned int)0x00000002,
}ELotusSocketFlag;

enum enmPacketToClientFlag
{
    ESCF_NO_PACKET_TO_CLIENT = 0, //Server并未给这个连接回过包
    ESCF_SENT_PACKET_TO_CLIENT = 1, //已经回过包
} ;

typedef struct tagExternalClientSocket
{
    //主机序
    int m_iSocketFD;

	//SessionID
	unsigned m_uiSessionFD;

    unsigned int m_uiSrcIP;
    unsigned short m_ushSrcPort;
    unsigned int m_uiCreateTime;
    int m_iSendFlag; //标志后台是否给这个客户回包了
    unsigned int m_uiRecvClientTimeStamp; //收到客户端数据的时间戳
    unsigned int m_uiRecvSvrTimeStamp; //收到Svr数据的时间戳

    //网络序
    unsigned int m_uiSocketFD_NBO;
    unsigned int m_uiSrcIP_NBO;
    unsigned short m_ushSrcPort_NBO;
    unsigned int m_uiCreateTime_NBO;

    unsigned int m_uiRecvEndOffset;
    char m_szRecvBuffer[EXTERNALCLIENT_RECVBUFFER_SIZE];

    unsigned int m_uiRecvPacketNumber;

    // Socket链表
    tagExternalClientSocket* m_pPrevSocket;
    tagExternalClientSocket* m_pNextSocket;
}TExternalClientSocket;

typedef struct tagAddress
{
	unsigned int m_uiIP;
	unsigned short m_ushPort;
}TAddress;

typedef struct tagAddressGroup
{
	int m_iNumber;
	TAddress m_astAddress[MAX_LISTENER_NUMBER];
}TAddressGroup;
