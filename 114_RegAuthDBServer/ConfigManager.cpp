
#include "AppDef.hpp"
#include "LogAdapter.hpp"

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
	//加载通信BUS配置
	int iRet = m_stBusConfigMgr.LoadServerBusConfig();
	if (iRet)
	{
		LOGERROR("Failed to load server bus config, ret %d\n", iRet);
		return iRet;
	}

	//加载RegAuthDB配置
	iRet = m_stRegAuthDBConfigManager.LoadDBConfig(REGAUTHDBINFO_CONFIG_FILE);
	if (iRet)
	{
		TRACESVR("Failed to load RegAuth db info config, ret %d\n", iRet);
		return iRet;
	}

	//加载uniqUinDB配置
	iRet = m_stUniqUinDBConfigManager.LoadDBConfig(UNIQUINDBINFO_CONFIG_FILE);
	if (iRet)
	{
		TRACESVR("Failed to load uniquin db info config, ret %d\n", iRet);
		return iRet;
	}

	return 0;
}

//获取通信BUS管理器
ServerBusConfigManager& ConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigMgr;
}

//获取RegAuthDB配置
const OneDBInfo* ConfigManager::GetRegAuthDBConfigByIndex(int iDBIndex)
{
	return m_stRegAuthDBConfigManager.GetDBConfigByIndex(iDBIndex);
}

//获取UniqUinDB配置
const OneDBInfo* ConfigManager::GetUniqUinDBConfigByIndex(int iDBIndex)
{
	return m_stUniqUinDBConfigManager.GetDBConfigByIndex(iDBIndex);
}
