
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "AppDef.hpp"

#include "ConfigManager.hpp"

using namespace ServerLib;

ConfigManager* ConfigManager::Instance()
{
	static ConfigManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new ConfigManager;
	}

	return pInstance;
}

ConfigManager::ConfigManager()
{

}

ConfigManager::~ConfigManager()
{
	m_stBusConfigMgr.Reset();
}

//加载配置
int ConfigManager::LoadAllConfig()
{
	//加载通信BUS
	int iRet = m_stBusConfigMgr.LoadServerBusConfig();
	if (iRet)
	{
		LOGERROR("Failed to load server bus config, ret %d\n", iRet);
		return -iRet;
	}

	//加载NameDB
	iRet = m_stNameDBConfigManager.LoadDBConfig(NAMEDBINFO_CONFIG_FILE);
	if (iRet)
	{
		TRACESVR("Failed to load name db config, ret %d\n", iRet);
		return iRet;
	}

	return 0;
}

//获取通信BUS管理器
ServerBusConfigManager& ConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigMgr;
}

//获取NameDB配置
const OneDBInfo* ConfigManager::GetNameDBConfigByIndex(int iDBIndex)
{
	return m_stNameDBConfigManager.GetDBConfigByIndex(iDBIndex);
}
	