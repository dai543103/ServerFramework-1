#pragma once

#include "EpollWrapper.hpp"
#include "ServerListener.hpp"
#include "FDPool.hpp"
#include "ConfigManager.hpp"
#include "OverloadSafeguard.hpp"
#include "RechargeProxy.h"
#include "YmnRechargeProxy.h"
#include "WechatRechargeProxy.h"

enum ENU_RESETERROR
{
    TIMEOUT_SVR_NOT_RSP = 1,                   // 空闲连接超时，且Server并未给这个连接回过包(closed by not having Server rsp)
    TIMEOUT_IDLE_CLIENT = 2,                   // 空闲client超时(closed by not recving message too long)
    TIMEOUT_IDLE_SVR    = 3,                   // 空闲server超时(closed by Server not rsp message too long)
    ERROR_EXTERAL_RECV_REQ_OVERLOAD = 8,       // OnReadFromExternal, RecvReqLimit Overload, Close Socket
    ERROR_EXTERAL_ACCEPT_RECV_OVERLOAD = 9,    // OnReadFromExternal, AcceptReceive Overload, Close Socket
    ERROR_EXTERAL_RECV = 10,                   // 接收失败或者玩家关闭
    ERROR_EXTERAL_RECV_PACKET_TOO_MORE = 11,   // 单个包长大于限制
    ERROR_EXTERAL_SEND_CLIENT_16 = 34,         // SendToExternalClient, Error Send
};

class CIOHandler
{
public:
	static CIOHandler* Instance();
	~CIOHandler();

    //初始化配置集合、
    int Initialize(bool bNagelOff);

    //创建网络IO处理器，本版本只支持Epoll
    int CreateEpoll();

    //创建所有监听器
    int CreateAllListener();

    //创建一枚监听器
    int CreateOneListener(unsigned int uiIP, unsigned short ushPort, unsigned int uiSendBufferSize);

    CServerListener* GetListenerByFD(int iListeningSocket);
    int ReCreateOneListener(CServerListener *pstLotusListener);

    //检查网络IO
    int CheckIO();

	//检查通信BUS
	int CheckBus();

    //检查超时情况
    int CheckTimeOut();
    int CheckOffline(); //检查断线

    int SendToExternalClient(unsigned uiSessionFD, const char* pszCodeBuffer, unsigned short ushCodeLength);

    int PrintStat();

public:
    //响应网络IO错误处理
    static int OnError(int iFD);

    //响应网络IO读
    static int OnRead(int iFD);

    //响应网络IO写
    static int OnWrite(int iFD);

    int AddFD(int iFD);

    //重置连接
    int Reset(int iFD, int iErrorNo, bool bNotSendOffline = false);

    //设置应用命令，停止服务、重载配置
    static void SetAppCmd(const int iAppCmd);

    //检查应用命令，停止服务、重载配置
    void CheckAppCmd();

    //停止服务
    void StopService();

    //重载配置
    void ReloadConfig();

private:
    //构造函数
	CIOHandler();

    //处理连接请求
    int Accept(int iListeningFD);

    //处理连接请求时的
    int OnAccept(unsigned int uiIP);

    //响应网络IO读，来自外部连接
    int OnReadFromExternal(int iFD);

	//加载日志配置
	int LoadLogConfig(const char* pszConfigFile, const char* pszLogName);

	//获取请求的URI
	void GetRequestURI(const char* pszCodeBuff, unsigned short usCodeLength, std::string& strPostURI);

	//处理BUS消息
	void ProcessBusMsg(const GameProtocolMsg& stMsg);

private:

    //FD池句柄
    CFDPool* m_pstFDPool;

    //网络IO处理器，本版本只支持Epoll实例
    CEpollWrapper m_stEpollWrapper;

    //监听器实例
    unsigned int m_uiListenerNumber;
    CServerListener m_astLotusListener[MAX_LISTENER_NUMBER];

    //应用命令
    static int m_iAppCmd;

    time_t m_tLastCheckTimeout; //上一次检查超时的时间

    //负载保护
    time_t m_tLastSafeguradTimeout;
    COverloadSafeguard m_stSafegurad; //过载保护类
    COverloadSafeguardStrategy m_stSafeguradStrategy; //过载保护策略类

    bool m_bNagelOff; // 收发套接字是否关闭nagel算法

	//接收数据
	int m_iRecvLen;
	char m_szRecvBuff[DEFAULT_RECVBUFFER_SIZE];

	//平台充值代理
	CRechargeProxy* m_apstRechargeProxy[LOGIN_PLATFORM_MAX];
};
