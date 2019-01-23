#include <fstream>
#include <sstream>

#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ConfigManager.hpp"
#include "SectionConfig.hpp"
#include "StringUtility.hpp"
#include "RegAuthPublic.hpp"

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
    int iRet = -1;
    CSectionConfig stConfigFile;
    iRet = stConfigFile.OpenFile("../conf/ServerList.tcm");
    if(iRet)
    {
        TRACESVR("Failed to get server list config file, ret %d\n", iRet);
        return -1;
    }

    //获取数量
    stConfigFile.GetItemValue("ServerList", "ServerNum", m_iServerInfoNum, 0);

    static char szConfigKey[64] = {0};
    for(int i=0; i<m_iServerInfoNum; ++i)
    {
        SAFE_SPRINTF(szConfigKey, sizeof(szConfigKey)-1, "ServerID_%d", i);
        stConfigFile.GetItemValue("ServerList", szConfigKey, m_astServerList[i].iServerID, 0);

        SAFE_SPRINTF(szConfigKey, sizeof(szConfigKey)-1, "ZoneServer_%d", i);
        stConfigFile.GetItemValue("ServerList", szConfigKey,  m_astServerList[i].szServerHostInfo, sizeof(m_astServerList[i].szServerHostInfo), "");
    }

    stConfigFile.CloseFile();

	//加载ServerBus配置
	iRet = m_stBusConfigMgr.LoadServerBusConfig();
	if (iRet)
	{
		LOGERROR("Failed to load server bus config, ret %d\n", iRet);
		return iRet;
	}

    return 0;
}

const ServerInfoConfig* CConfigManager::GetServerInfo(int iServerID)
{
    for(int i=0; i<m_iServerInfoNum; ++i)
    {
        if(m_astServerList[i].iServerID == iServerID)
        {
            return &m_astServerList[i];
        }
    }

    return NULL;
}

//获取通信BUS管理器
ServerBusConfigManager& CConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigMgr;
}
