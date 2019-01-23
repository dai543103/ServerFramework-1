#include <fstream>
#include <sstream>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ConfigManager.hpp"
#include "SectionConfig.hpp"
#include "StringUtility.hpp"
#include "ExchangePublic.hpp"

using namespace std;
using namespace ServerLib;

CConfigManager* CConfigManager::Instance()
{
	static CConfigManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CConfigManager;
	}

	return pInstance;
}

CConfigManager::~CConfigManager()
{
	m_stBusConfigMgr.Reset();
}

CConfigManager::CConfigManager()
{

}

int CConfigManager::LoadAllConf()
{
	//加载ServerBus配置
	int iRet = m_stBusConfigMgr.LoadServerBusConfig();
	if (iRet)
	{
		LOGERROR("Failed to load server bus config, ret %d\n", iRet);
		return iRet;
	}

	//加载CardNoDB
	iRet = m_stCardNoDBConfigManager.LoadDBConfig(CARDNODBINFO_CONFIG_FILE);
	if (iRet)
	{
		TRACESVR("Failed to load cardno db config, ret %d\n", iRet);
		return iRet;
	}

    return 0;
}

//获取通信BUS管理器
ServerBusConfigManager& CConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigMgr;
}

//获取LogDB配置
const OneDBInfo* CConfigManager::GetCardNoDBConfigByIndex(int iDBIndex)
{
	return m_stCardNoDBConfigManager.GetDBConfigByIndex(iDBIndex);
}
