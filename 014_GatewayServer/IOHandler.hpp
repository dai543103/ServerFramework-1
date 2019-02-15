#ifndef __IO_HANDLER_HPP__
#define __IO_HANDLER_HPP__

#include "EpollWrapper.hpp"
#include "LotusListener.hpp"
#include "FDPool.hpp"
#include "CodeQueueAssemble.hpp"
#include "ConfigAssemble.hpp"
#include "CodeDispatcher.hpp"
#include "OverloadSafeguard.hpp"

enum ENU_RESETERROR
{
    TIMEOUT_SVR_NOT_RSP = 1,                   // �������ӳ�ʱ����Server��δ��������ӻع���(closed by not having Server rsp)
    TIMEOUT_IDLE_CLIENT = 2,                   // ����client��ʱ(closed by not recving message too long)
    TIMEOUT_IDLE_SVR    = 3,                   // ����server��ʱ(closed by Server not rsp message too long)
    ERROR_NET_IO        = 4,                   // ��Ӧ����IO������
    ERROR_INTERNAL_CONN = 5,                   // �ڲ������׽���(����ر�)
    ERROR_INTERNAL_IO_READ = 6,                // ��Ӧ����IO���������ڲ�����,Recv < 0
    ERROR_DISPATCH_FROM_INTERNAL_SVR = 7,      // DispatchFromInternalServer return < 0
    ERROR_EXTERAL_RECV_REQ_OVERLOAD = 8,       // OnReadFromExternal, RecvReqLimit Overload, Close Socket
    ERROR_EXTERAL_ACCEPT_RECV_OVERLOAD = 9,    // OnReadFromExternal, AcceptReceive Overload, Close Socket
    ERROR_EXTERAL_RECV = 10,                   // ����ʧ�ܻ�����ҹر�
    ERROR_EXTERAL_RECV_PACKET_TOO_MORE = 11,   // ����������������
    RSP_FLASH_STRATEGY_FILE = 12,              // �·�Flash�����ļ�OK   
    RSP_U3D_STRATEGY_FILE = 13,                // �·�U3D�����ļ�OK
    ERROR_DISPATCH_FROM_EXTERNAL_CLIENT = 14,  // OnReadFromExternal, DispatchFromExternalClient fail
    SVR_NOTIFY_CLOSE_FD = 15,                  // SendToExternalClient, Svr Notify TO Close Socket
    ERROR_EXTERAL_SEND_TO_CLIENT_FAIL = 16,    // SendToExternalClient Error Send
    ERROR_PLAYERNOTIFYTCPCLOSESOCKET = 17,           // SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET
    ERROR_EXTERAL_SEND_CLIENT_1 = 18,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_2 = 19,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_3 = 20,          // SendToExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_4 = 21,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_5 = 22,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_6 = 23,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_7 = 24,          // SendToAllExternalClient, Error Send
    ERROR_EXTERAL_SEND_CLIENT_8 = 25,          // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_9 = 26,          // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_10 = 27,         // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_11 = 28,         // SendToAllExternalClientButOne, Error Send
    ERROR_EXTERAL_SEND_CLIENT_12 = 29,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_13 = 30,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_14 = 31,         // SendToExternalClientList, Error Send
    ERROR_EXTERAL_SEND_CLIENT_15 = 32,         // SendToExternalClientList, Error Send
    PLAYERNOTIFYTCPCLOSESOCKET_2 = 33,         // SendToExternalClient, PLAYERNOTIFYTCPCLOSESOCKET
    ERROR_EXTERAL_SEND_CLIENT_16 = 34,         // SendToExternalClient, Error Send
    ERROR_AUTH_FAILED_WHILE_Initing = 35,      // ProcessAuthCodeQueue, Auth Failed While Initing, Reset
    INVALID_AUTHORIZE_PACKET = 36,             // DispatchOneCode, Invalid Authorized packet
    SEND_TO_AUTH_FAIL = 37,                    // Send To Auth failed
    SEND_TO_INTERNAL_SVR_FAIL = 38,            // DispatchToDstServer,SendToInternalServer fail
};

class CIOHandler
{
public:

    //��ʼ�����ü��ϡ�
    int Initialize(bool bNagelOff);

    //��������IO�����������汾ֻ֧��Epoll
    int CreateEpoll();

    //�������м�����
    int CreateAllListener();

    //����һö������
    int CreateOneListener(unsigned int uiIP, unsigned short ushPort,
        unsigned int uiSendBufferSize, bool bExternalClient);

    CLotusListener* GetListenerByFD(int iListeningSocket);
    int ReCreateOneListener(CLotusListener *pstLotusListener);

    //�������IO
    int CheckIO();

    //��鳬ʱ���
    int CheckTimeOut();
    int CheckOffline(); //������

    int DispatchFromInternalServer(int iSrcFD);

    //������ͨ��CodeQueue
    int CheckCodeQueue();

    int CheckDefaultCodeQueue();

    int SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBuffer);

    int SendToExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg, const char* pszCodeBufEnd,
        const char* pszHeadCode, const char* pszTailCode);
    int SendToExternalClientList(const unsigned short ushCodeLength, const char* pszCodeBufBeg, const char* pszCodeBufEnd,
        const char* pszHeadCode, const char* pszTailCode);
    int SendToAllExternalClient(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
        const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode);
    int SendToAllExternalClientButOne(const unsigned short ushCodeLength, const char* pszCodeBufBeg,
        const char* pszCodeBufEnd, const char* pszHeadCode, const char* pszTailCode);

    int PrintStat();

    CCodeQueueAssemble& GetCodeQueueAssemble() {return m_stCodeQueueAssemble;}

public:
    //��Ӧ����IO������
    static int OnError(int iFD);
    //��Ӧ����IO��
    static int OnRead(int iFD);
    //��Ӧ����IOд
    static int OnWrite(int iFD);

    int AddFD(int iFD);

    //��������
    int Reset(int iFD, int iErrorNo, bool bNotSendOffline = false);

    //����Ӧ�����ֹͣ������������
    static void SetAppCmd(const int iAppCmd);

    //���Ӧ�����ֹͣ������������
    void CheckAppCmd();

    //ֹͣ����
    void StopService();

    //��������
    void ReloadConfig();

    //֪ͨ����
    int NotifyInternalMsg(int iFD, unsigned int uiUin, unsigned short ushCtrlFlag = 0);

    //�Ͽ�Server���øı���ڲ�����
    int ResetInternalServer();

private:
    // �������ݰ�����
    int DispatchFromExternalClient(int iFD);

    //������������
    int Accept(int iListeningFD);

    //������������ʱ��
    int OnAccept(unsigned int uiIP);

    //��Ӧ����IO���������ڲ�����
    int OnReadFromInternal(int iFD);

    //��Ӧ����IO���������ⲿ����
    int OnReadFromExternal(int iFD);

private:

    //FD�����õĹ����ڴ�ʵ��
    CSharedMemory m_stSharedMemoryFDPool;

    //FD�ؾ��
    CFDPool* m_pstFDPool;

    //����IO�����������汾ֻ֧��Epollʵ��
    CEpollWrapper m_stEpollWrapper;

    //������ʵ��
    unsigned int m_uiListenerNumber;
    CLotusListener m_astLotusListener[MAX_LISTENER_NUMBER];

    //CodeQueue����
    CCodeQueueAssemble m_stCodeQueueAssemble;

    //��������
    CConfigAssemble m_stConfigAssemble;

    //��Ϣ�ɷ���
    CCodeDispatcher m_stCodeDispatcher;

    //Ӧ������
    static int m_iAppCmd;

    time_t m_tLastCheckTimeout; //��һ�μ�鳬ʱ��ʱ��
    time_t m_tLastCheckHello;//�ϴμ��Helloʱ��

    //���ر���
    time_t m_tLastSafeguradTimeout;
    COverloadSafeguard m_stSafegurad; //���ر�����
    COverloadSafeguardStrategy m_stSafeguradStrategy; //���ر���������

private:

    static CIOHandler* m_pstThisHandler;
    bool m_bNagelOff; // �շ��׽����Ƿ�ر�nagel�㷨

};





#endif
