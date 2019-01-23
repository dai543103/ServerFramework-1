#pragma once

#include "AppDefine.hpp"
#include "ServerBusConfigManager.h"

class CConfigManager
{
public:
	static CConfigManager* Instance();
	~CConfigManager();

public:

	//加载配置
	int LoadAllConfig();

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

    int GetIdleConnectionTimeout();
    int GetIdleClientTimeout();
    int GetIdleServerTimeout();
    int GetClientPacketMaxFrequencyNum();
	int GetExternalRCVBufferSize();
	int GetExternalSNDBufferSize();
	int GetClientPacketMaxLength();
    const TAddressGroup& GetExternalListenerAddress();

private:
	CConfigManager();

	//加载服务配置
	int LoadServerConfig();

private:

    // 接收缓冲区大小
	int m_iExternalSocketRCVBufferSize;

    // 发送缓冲区大小
	int m_iExternalSocketSNDBufferSize;

    //单次上行报文长度上限
	 int m_iClientPacketMaxLength;

    //检查超时的时间
    int m_iIdleConnectionTimeout; //连接超时时间
    int m_iIdleClientTimeout; //没有收到客户端包超时时间
    int m_iIdleServerTimeout; //没有收到Svr回包的超时时间

    //上行请求频率限制
    int m_iClientPacketMaxFrequency;

    //外部监听地址集合
    TAddressGroup m_stClientListenerAddress;

	//BUS配置管理器
	ServerBusConfigManager m_stBusConfigManager;
};
