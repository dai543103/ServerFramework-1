#ifndef __LOTUS_DEFINE_HPP__
#define __LOTUS_DEFINE_HPP__



//TCP���ӿ�����Ϣ
#define PLAYERNOTIFYTCPCLOSESOCKET  1

const int APPCMD_STOP_SERVICE = 1;
const int APPCMD_RELOAD_CONFIG = 2;

#ifdef _DEBUG_
const unsigned int FD_SIZE = 5000; //�������
#else
const unsigned int FD_SIZE = 50000;
#endif

const unsigned int EXTERNALCLIENT_RECVBUFFER_SIZE = 4096;

const unsigned int INTERNALSERVER_RECVBUFFER_SIZE = 2048000;
const unsigned int INTERNALSERVER_SENDBUFFER_SIZE = 2048000;

const unsigned int DISPATCHING_BUFFER_SIZE = 64000;

const unsigned int MAX_VALIDCODE_LENGTH = 32000;

const unsigned int MAX_LISTENER_NUMBER = 16;

const unsigned int PROXYHEAD_BASELENGTH = 14;


const unsigned int MAX_NETHEAD_SIZE = 64;
const unsigned int MIN_NETHEAD_SIZE = 16;

const unsigned int NETHEAD_V2_SIZE = 24;

const unsigned int CRYPT_KEY_LENGTH = 16;

typedef enum enLotusSocketFlag
{
    ELSF_ListeningForExternalClient = (unsigned int)0x00000001,
    ELSF_ListeningForInternalServer = (unsigned int)0x00000002,
    ELSF_ConnectedByExternalClient = (unsigned int)0x00000010,
    ELSF_ConnectedByInternalServer = (unsigned int)0x00000020,
}ELotusSocketFlag;

enum enmPacketToClientFlag
{
    ESCF_NO_PACKET_TO_CLIENT = 0, //Server��δ��������ӻع���
    ESCF_SENT_PACKET_TO_CLIENT = 1, //�Ѿ��ع���
} ;

typedef struct tagExternalClientSocket
{
    //������
    int m_iSocketFD;

    unsigned int m_uiSrcIP;
    unsigned short m_ushSrcPort;
    unsigned int m_uiCreateTime;
    int m_iSendFlag; //��־��̨�Ƿ������ͻ��ذ���
    unsigned int m_uiRecvClientTimeStamp; //�յ��ͻ������ݵ�ʱ���
    unsigned int m_uiRecvSvrTimeStamp; //�յ�Svr���ݵ�ʱ���

    //������
    unsigned int m_uiSocketFD_NBO;
    unsigned int m_uiSrcIP_NBO;
    unsigned short m_ushSrcPort_NBO;
    unsigned int m_uiCreateTime_NBO;

    unsigned int m_uiRecvEndOffset;
    char m_szRecvBuffer[EXTERNALCLIENT_RECVBUFFER_SIZE];

    unsigned int m_uiUin; //����֪ͨ���ߵ�ʱ����Ҫ����Uin����������Ҫ�����ⲿ���ӵ�Uin

    unsigned int m_uiRecvPacketNumber;

    // Socket����
    tagExternalClientSocket* m_pPrevSocket;
    tagExternalClientSocket* m_pNextSocket;
}TExternalClientSocket;

typedef struct tagInternalServerSocket
{
    //������
    int m_iSocketFD;

    unsigned int m_uiSrcIP;
    unsigned short m_ushListenedPort;

    unsigned short m_ushServerType;
    unsigned short m_ushServerID;

    unsigned int m_uiRecvEndOffset;
    char m_szRecvBuffer[INTERNALSERVER_RECVBUFFER_SIZE];

    unsigned int m_uiSendEndOffset;
    char m_szSendBuffer[INTERNALSERVER_SENDBUFFER_SIZE];

}TInternalServerSocket;

typedef enum enmReserveCtrlFlag
{
    ERCF_LOTUS_NOTIFY_CLIENT_OFFLINE = 1,

} ENMCODECTRLFLAG;

//�ڲ�����ϢID����100���µ�ID����Ҫ�����ڵ���ϢID�ظ�
typedef enum enmInternalMsgID
{
    EIMI_LOTUS_NOTIFY_OFFLINE = 1,
} ENMINTERNALMSGID;

typedef struct tagNetHead_V2
{
    unsigned int    m_uiSocketFD;   //�׽���
    unsigned int    m_uiSocketTime; //�׽��ִ���ʱ��
    unsigned int    m_uiSrcIP;  //Դ��ַ
    unsigned short  m_ushSrcPort;   //Դ�˿�
    unsigned short  m_ushReservedValue01;   //�ֽڶ��룬δ��
    unsigned int    m_uiCodeTime;   //��Ϣʱ��
    unsigned int    m_uiCodeFlag;   //��Ϣ��־������ʵ���׽��ֿ���
}TNetHead_V2;
#endif
