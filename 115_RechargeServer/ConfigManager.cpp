
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "SectionConfig.hpp"
#include "LogAdapter.hpp"
#include "AppDefine.hpp"

#include "ConfigManager.hpp"

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

CConfigManager::CConfigManager()
{
    m_iClientPacketMaxLength = (int)DEFAULT_INPUTPACKET_LENGTHRESTRCIT;
    m_iIdleConnectionTimeout = 0;
    m_iIdleClientTimeout = 0;
    m_iIdleServerTimeout = 0;
    m_iClientPacketMaxFrequency = 0;
    m_stClientListenerAddress.m_iNumber = 0;
}

CConfigManager::~CConfigManager()
{

}

//加载配置
int CConfigManager::LoadAllConfig()
{
	int iRet = LoadServerConfig();
	if (iRet)
	{
		LOGERROR("Failed to laod server config, ret %d\n", iRet);
		return -1;
	}

	iRet = m_stBusConfigManager.LoadServerBusConfig();
	if (iRet)
	{
		LOGERROR("Failed to load bus config, ret %d\n", iRet);
		return -2;
	}

	return 0;
}

//获取通信BUS管理器
ServerBusConfigManager& CConfigManager::GetBusConfigMgr()
{
	return m_stBusConfigManager;
}

const TAddressGroup& CConfigManager::GetExternalListenerAddress()
{
    return m_stClientListenerAddress;
}

int CConfigManager::GetIdleConnectionTimeout()
{
    return m_iIdleConnectionTimeout;
}

int CConfigManager::GetIdleClientTimeout()
{
    return m_iIdleClientTimeout;
}

int CConfigManager::GetIdleServerTimeout()
{
    return m_iIdleServerTimeout;
}

int CConfigManager::GetExternalRCVBufferSize()
{
    return m_iExternalSocketRCVBufferSize;
}

int CConfigManager::GetExternalSNDBufferSize()
{
    return m_iExternalSocketSNDBufferSize;
}

int CConfigManager::GetClientPacketMaxLength()
{
    return m_iClientPacketMaxLength;
}

int CConfigManager::GetClientPacketMaxFrequencyNum()
{
    return m_iClientPacketMaxFrequency;
}

//加载服务配置
int CConfigManager::LoadServerConfig()
{
	CSectionConfig stConfigFile;
	int iRet = stConfigFile.OpenFile(APP_CONFIG_FILE);
	if (iRet)
	{
		LOGERROR("Open config file %s failed.\n", APP_CONFIG_FILE);
		return -1;
	}

	int iSendBufferSize = 0;
	stConfigFile.GetItemValue("Network", "ExternalSocketRCVBufferSize", iSendBufferSize, DEFAULT_SENDBUFFER_SIZE);
	m_iExternalSocketRCVBufferSize = iSendBufferSize;

	stConfigFile.GetItemValue("Network", "ExternalSocketSNDBufferSize", iSendBufferSize, DEFAULT_SENDBUFFER_SIZE);
	m_iExternalSocketSNDBufferSize = iSendBufferSize;

	stConfigFile.GetItemValue("Network", "ClientPacketMaxLength", m_iClientPacketMaxLength, DEFAULT_INPUTPACKET_LENGTHRESTRCIT);
	stConfigFile.GetItemValue("Network", "IdleConnectionTimeout", m_iIdleConnectionTimeout, 30);
	stConfigFile.GetItemValue("Network", "IdleClientTimeout", m_iIdleClientTimeout, 180);
	stConfigFile.GetItemValue("Network", "IdleServerTimeout", m_iIdleServerTimeout, 1200);
	stConfigFile.GetItemValue("Network", "ClientPacketMaxFrequency", m_iClientPacketMaxFrequency, 1000);

	LOGDEBUG("ExternalRCVBufferSize:%u ExternalSNDBufferSize:%d  ClientPacketMaxLength:%d\n",
		m_iExternalSocketRCVBufferSize, m_iExternalSocketSNDBufferSize, m_iClientPacketMaxLength);

	//读取监听IP端口信息
	stConfigFile.GetItemValue("Network", "Listening_HostNum", m_stClientListenerAddress.m_iNumber, 8);

	char szHostInfo[64] = { 0 };
	char szKey[64] = { 0 };
	char szHostIP[32] = { 0 };
	int iRetLen = 0;
	for (int i = 0; i < m_stClientListenerAddress.m_iNumber; ++i)
	{
		sprintf(szKey, "Listening_Host_%d", i);

		iRetLen = stConfigFile.GetItemValue("Network", szKey, szHostInfo, sizeof(szHostInfo) - 1);
		if (iRetLen == 0)
		{
			//没有新配置
			break;
		}

		TAddress& stAddress = m_stClientListenerAddress.m_astAddress[i];

		//IP:PORT
		sscanf(szHostInfo, "%[^:]:%hu", szHostIP, &stAddress.m_ushPort);
		stAddress.m_uiIP = inet_addr(szHostIP);

		LOGDEBUG("ClientListener[%d] IP:%u Port:%u\n", i, stAddress.m_uiIP, stAddress.m_ushPort);
	}

	stConfigFile.CloseFile();
	return 0;
}
