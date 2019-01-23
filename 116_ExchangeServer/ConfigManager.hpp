#pragma once

#include "ServerBusConfigManager.h"
#include "DBConfigManager.h"

class CConfigManager
{
public:

	static CConfigManager* Instance();
	~CConfigManager();

    int LoadAllConf();
public:

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

	//获取CardNoDB配置
	const OneDBInfo* GetCardNoDBConfigByIndex(int iDBIndex);

private:
	CConfigManager();

private:

	//通信BUS配置管理器
	ServerBusConfigManager m_stBusConfigMgr;

	//CardNoDB信息配置
	DBConfigManager m_stCardNoDBConfigManager;
};
