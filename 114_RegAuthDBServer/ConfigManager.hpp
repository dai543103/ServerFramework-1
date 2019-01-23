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

	//获取RegAuthDB配置
	const OneDBInfo* GetRegAuthDBConfigByIndex(int iDBIndex);

	//获取UniqUinDB配置
	const OneDBInfo* GetUniqUinDBConfigByIndex(int iDBIndex);

private:
	ConfigManager();

private:
	//通信BUS配置管理器
	ServerBusConfigManager m_stBusConfigMgr;

	//RegAuthDB配置
	DBConfigManager m_stRegAuthDBConfigManager;

	//UniqUinDB配置，用于分配uin
	DBConfigManager m_stUniqUinDBConfigManager;
};

