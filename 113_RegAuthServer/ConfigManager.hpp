#ifndef __CONFIG_MGR_H__
#define __CONFIG_MGR_H__

#include "ServerBusConfigManager.h"

struct ServerInfoConfig
{
    int iServerID;
    char szServerHostInfo[32];

    ServerInfoConfig()
    {
        memset(this, 0, sizeof(*this));
    };
};

class CConfigManager
{
public:

	static CConfigManager* Instance();
	~CConfigManager();

    int LoadAllConf();
public:
    const ServerInfoConfig* GetServerInfo(int iServerID);

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

private:
	CConfigManager();

private:
    //游戏服务器列表
    int m_iServerInfoNum;
    ServerInfoConfig m_astServerList[32];

	//通信BUS配置管理器
	ServerBusConfigManager m_stBusConfigMgr;
};

#endif // __CONFIG_MGR_H__
