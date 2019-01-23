
#include <errno.h>

#include "SectionConfig.hpp"
#include "Kernel/AppDef.hpp"
#include "Kernel/ModuleHelper.hpp"

#include "ConfigManager.hpp"

static const char* sModuleName[MODULE_MAX_ID] =
{
	"IsFishEnabled",
};

CConfigManager* CConfigManager::Instance()
{
	static CConfigManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CConfigManager;
	}

	return pInstance;
}

CConfigManager::CConfigManager()
{

}

CConfigManager::~CConfigManager()
{

}

int CConfigManager::Initialize(bool bResumeMode)
{
    int iRet = 0;

    //GM权限相关配置
    iRet = m_stGMPrivConfigManager.LoadGMPrivConfig();
    if(iRet)
    {
        return -1;
    }

	//通信BUS配置管理器
	iRet = m_stBusConfigManager.LoadServerBusConfig();
	if (iRet)
	{
		return -2;
	}

	//Excel配置管理器
	iRet = m_stBaseConfigManager.LoadAllConfig("../../../../Common/config/");
	if (iRet)
	{
		return -3;
	}

	//加载服务器配置
	iRet = LoadZoneConfig();
	if (iRet)
	{
		return -4;
	}

	//先清空原有消息配置里信息
	CHandlerFactory::ClearAllDisableMsg();

	//load
	LoadMsgInterval();
	LoadMsgDiableList();

    // 打印配置类内存使用情况
    PrintConfigMemoryStatics();

    return 0;
};

//GM权限配置
CGMPrivConfigManager& CConfigManager::GetGMPrivConfigManager()
{
	return m_stGMPrivConfigManager;
}

//获取通信BUS管理器
ServerBusConfigManager& CConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigManager;
}

//获取基础配置管理器
BaseConfigManager& CConfigManager::GetBaseCfgMgr()
{
	return m_stBaseConfigManager;
}

//模块是否关闭
bool CConfigManager::IsModuleSwitchOn(MODULE_ID eModuleID)
{
	return m_aiModuleSwitch[(int)eModuleID];
}

//是否检查SessionKey
bool CConfigManager::IsSSKeyCheckEnabled()
{
	return m_iCheckSSKeyEnabled;
}

// 是否开启Player日志
bool CConfigManager::IsPlayerLogEnabled()
{
	return m_iIsEnablePlayerLog;
}

//加载服务器配置
int CConfigManager::LoadZoneConfig()
{
	CSectionConfig stConfigFile;
	int iRet = stConfigFile.OpenFile(APP_CONFIG_FILE);
	if (iRet)
	{
		LOGERROR("Failed to load config file %s\n", APP_CONFIG_FILE);
		return -1;
	}

	//加载模块开关
	memset(m_aiModuleSwitch, 0, sizeof(m_aiModuleSwitch));
	for (int i = 0; i < (int)MODULE_MAX_ID; ++i)
	{
		stConfigFile.GetItemValue("ModuleSwitch", sModuleName[i], m_aiModuleSwitch[i], 1);
	}

	//加载SessionKey检查开关
	stConfigFile.GetItemValue("module", "SSKeyChecked", m_iCheckSSKeyEnabled, 0);

	//加载打印Player日志开关
	stConfigFile.GetItemValue("Param", "IsEnablePlayerLog", m_iIsEnablePlayerLog, 0);

	stConfigFile.CloseFile();

	return 0;
}

int CConfigManager::PrintConfigMemoryStatics()
{
    LOGDEBUG("********************** CONFIG MEMORY STATICS *********************\n");

    int iConfigTotalSize = 0;

    // 总计
    LOGDEBUG("Total Memory: %dB, %dMB\n", iConfigTotalSize, iConfigTotalSize/1024/1024);
    LOGDEBUG("*****************************************************************\n");

    return 0;
}

int CConfigManager::LoadMsgInterval()
{
    CSectionConfig stConfigFile;
    int iRet = stConfigFile.OpenFile(APP_CONFIG_FILE);
    if (iRet != 0)
    {
        return 0;
    }

    char szMsgLimitFile[MAX_FILENAME_LENGTH];
    stConfigFile.GetItemValue("MsgLimit", "MsgLimitFile", szMsgLimitFile, sizeof(szMsgLimitFile), "../conf/MsgLimitFile.conf");

    FILE* fp = fopen(szMsgLimitFile, "r");
    if (fp == NULL)
    {
        TRACESVR("can not open MsgLimitFile:%s for :%s\n", szMsgLimitFile, strerror(errno));
        printf("can not open MsgLimitFile:%s for: %s\n", szMsgLimitFile, strerror(errno));
        return -1;
    }

    char szLine[100];
    unsigned int uiMsgID;
    unsigned int uiMsgLimitInterval;
    while(fgets(szLine, sizeof(szLine), fp))
    {
        //空行或者格式不对
        if (sscanf(szLine, "%u %u", &uiMsgID, &uiMsgLimitInterval) != 2)
        {
            continue;
        }
        if (uiMsgID >= (unsigned)MAX_HANDLER_NUMBER)
        {
            TRACESVR("file :%s msg id is too bigger than MAX_HANDLER_NUMBER", szMsgLimitFile);
            fclose(fp);
            return -20;
        }

        if (uiMsgLimitInterval >= (unsigned int)MAX_MSG_INTERVAL_TIME)
        {
            uiMsgLimitInterval = MAX_MSG_INTERVAL_TIME;
        }

        TZoneMsgConfig* pstMsgConfig = CHandlerFactory::GetMsgConfig(uiMsgID);
        if (!pstMsgConfig)
        {
            continue;
        }

        TRACESVR("msgid:%u  MsgLimitInterval:%u\n", uiMsgID, uiMsgLimitInterval);
        pstMsgConfig->m_iMsgInterval = uiMsgLimitInterval;
    }

    fclose(fp);

    return 0;
}

int CConfigManager::LoadMsgDiableList()
{
    CSectionConfig stConfigFile;
    int iRet = stConfigFile.OpenFile(APP_CONFIG_FILE);
    if (iRet != 0)
    {
        return 0;
    }

    char szMsgDisableFile[MAX_FILENAME_LENGTH];
    stConfigFile.GetItemValue("MsgDisable", "MsgDisableFile", szMsgDisableFile, sizeof(szMsgDisableFile), "../conf/MsgDisableFile.conf");

    //没有就跳过
    FILE* fp = fopen(szMsgDisableFile, "r");
    if (fp == NULL)
    {
        TRACESVR("can not open MsgLimitFile:%s for :%s\n", szMsgDisableFile, strerror(errno));
        return 0;
    }

    char szLine[100];
    unsigned int uiMsgID;
    while(fgets(szLine, sizeof(szLine), fp))
    {
        //空行或者格式不对
        if (sscanf(szLine, "%u", &uiMsgID) != 1)
        {
            continue;
        }

        CHandlerFactory::DisabledMsg(uiMsgID);
        TRACESVR("msgid:%u is disabled", uiMsgID);
    }

    fclose(fp);

    return 0;
}

int CConfigManager::GetMsgInterval(unsigned int uiMsgID)
{
    if(uiMsgID >= (unsigned)MAX_HANDLER_NUMBER)
    {
        TRACESVR("NULL handler, msg id: %d\n", uiMsgID);
        return 0;
    }

    TZoneMsgConfig* pstMsgConfig = CHandlerFactory::GetMsgConfig(uiMsgID);
    if (!pstMsgConfig)
    {
        return 0;
    }

    return pstMsgConfig->m_iMsgInterval;
}
