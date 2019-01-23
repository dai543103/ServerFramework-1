#pragma once

#include "ServerBusConfigManager.h"
#include "DBConfigManager.h"

//配置管理器
class ConfigManager
{
public:
	static ConfigManager* Instance();
	~ConfigManager();

	//加载配置
	int LoadAllConfig();

	//获取通信BUS管理器
	ServerBusConfigManager& GetBusConfigMgr();

	//获取MofangDB配置
	const OneDBInfo* GetLogDBConfigByIndex(int iDBIndex);

private:
	ConfigManager();

private:
	//通信BUS配置管理器
	ServerBusConfigManager m_stBusConfigMgr;

	//LogDB信息配置
	DBConfigManager m_stLogDBConfigManager;
};

