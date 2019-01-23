#pragma once

#include "ServerBusConfigManager.h"
#include "BaseConfigManager.hpp"

typedef struct
{
    char m_szHostIP[24];
    unsigned short m_ushPort;
    unsigned short m_ushZoneID;
    char m_szZoneName[64];
} TZoneConf;

typedef struct
{
    unsigned short m_ushLen;

    // 保存网通和电信2个IP信息
    TZoneConf m_astZoneConf[MAX_ZONE_PER_WORLD*2];
} TZoneConfList;

class CConfigManager
{
public:
	static CConfigManager* Instance();
	~CConfigManager();

    int Initialize(bool bResume);

    int LoadZoneConf(const char* pszConfFile);
	const TZoneConfList& GetZoneConfList();

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

	//获取基础配置管理器
	BaseConfigManager& GetBaseCfgMgr();

	//获取大区满玩家数量
	int GetFullWorldNum();

	//获取大区繁忙玩家数量
	int GetBusyWorldNum();

private:
	CConfigManager();

private:

	//分区列表信息
    TZoneConfList m_stZoneConfList;
	
	//服务器通信BUS配置
	ServerBusConfigManager m_stBusConfigMgr;

	//服务器基础配置
	BaseConfigManager m_stBaseConfigManager;

	//大区满支持的玩家数量
	int m_iFullWorldNum;

	//大区繁忙支持的玩家数量
	int m_iBusyWorldNum;
};
