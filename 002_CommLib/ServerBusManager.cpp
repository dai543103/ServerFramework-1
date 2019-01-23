
#include "GameProtocol.hpp"
#include "Int64Utility.hpp"
#include "LogAdapter.hpp"
#include "StringUtility.hpp"

#include "ServerBusManager.h"

using namespace ServerLib;

static const char* szCodeQueueConfigFile = "../conf/GameServer.tcm";

CServerBusManager* CServerBusManager::Instance()
{
	static CServerBusManager* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CServerBusManager;
	}

	return pInstance;
}

CServerBusManager::CServerBusManager()
{
	m_iRecvBusIndex = 0;

	//内网用ZMQBUS通信
	m_iBusNum = 0;
}

CServerBusManager::~CServerBusManager()
{

}

//初始化通信BUS
int CServerBusManager::Init(const char* pszServerName, ServerBusConfigManager& stBusConfigMgr)
{
	m_iRecvBusIndex = 0;

	// 初始化 Lotus 的CodeQueue
	int iRet = m_stCodeQueueMgr.LoadCodeQueueConfig(szCodeQueueConfigFile, pszServerName);
	if (iRet < 0)
	{
		TRACESVR("LoadCodeQueueConfig failed, iRet:%d\n", iRet);
		return -1;
	}

	m_stCodeQueueMgr.Initialize(false);
	if (iRet < 0)
	{
		TRACESVR("Code Queue Manager Initialize failed, iRet:%d\n", iRet);
		return -2;
	}

	//初始化ZMQ通信BUS
	const std::vector<ServerBusConfig>& vBusConfigs = stBusConfigMgr.GetAllServerBus();

	m_iBusNum = vBusConfigs.size();
	for (int i = 0; i < m_iBusNum; ++i)
	{
		const ServerBusConfig& stConfig = vBusConfigs[i];
		iRet = m_astServerBus[i].ZmqInit(stConfig.ullServerBusID, stConfig.szBusAddr, EN_ZMQ_SOCKET_PAIR, EN_ZMQ_PROC_TCP, stConfig.bIsServerEnd);
		if (iRet)
		{
			TRACESVR("Fail to init zmq bus, bus id %s, ret %d\n", GetBusString(stConfig.ullServerBusID).c_str(), iRet);
			return iRet;
		}
	}

	return 0;
}

//发送消息
int CServerBusManager::SendOneMsg(const char* pszMsg, int iMsgLength, const SERVERBUSID& stToBusID)
{
	ASSERT_AND_LOG_RTN_INT(pszMsg);

	int iRet = 0;

	switch (stToBusID.usServerID)
	{
	case GAME_SERVER_LOTUS:
	{
		iRet = m_stCodeQueueMgr.SendOneMsg(pszMsg, iMsgLength, stToBusID.usInstanceID);
	}
	break;

	case GAME_SERVER_WORLD:
	case GAME_SERVER_ZONE:
	case GAME_SERVER_ROLEDB:
	case GAME_SERVER_REGAUTH:
	case GAME_SERVER_REGAUTHDB:
	case GAME_SERVER_NAMEDB:
	case GAME_SERVER_RECHARGE:
	case GAME_SERVER_EXCHANGE:
	case GAME_SERVER_LOGSERVER:
	{
		uint64_t ullServerBusID = GenerateBusID(stToBusID.usWorldID, (EGameServerID)stToBusID.usServerID, stToBusID.usInstanceID, stToBusID.usZoneID);
		ZmqBus* pstServerBus = GetZmqBus(ullServerBusID);
		if (!pstServerBus)
		{
			return -1;
		}

		iRet = pstServerBus->ZmqSend(pszMsg, iMsgLength, 0);
	}
	break;

	default:
	{
		TRACESVR("Bug..................................\n");
		break;
	}
	}

	return iRet;
}

//接收消息
int CServerBusManager::RecvOneMsg(char* pszMsg, int iMaxOutMsgLen, int& riMsgLength, SERVERBUSID& stFromBusID)
{
	ASSERT_AND_LOG_RTN_INT(pszMsg);

	int iRet = 0;
	int iCodeQueueNum = m_stCodeQueueMgr.GetCodeQueueNum();
	unsigned short usMsgPeer = stFromBusID.usServerID;

	if (usMsgPeer == GAME_SERVER_LOTUS)
	{
		//指定接收CodeQueue
		if (m_iRecvBusIndex >= iCodeQueueNum)
		{
			m_iRecvBusIndex = 0;
		}
	}
	else if (usMsgPeer != GAME_SERVER_UNDEFINE)
	{
		//指定接收ZMQ BUS
		if (m_iRecvBusIndex < iCodeQueueNum)
		{
			m_iRecvBusIndex = iCodeQueueNum;
		}
	}
	
	if (m_iRecvBusIndex < iCodeQueueNum)
	{
		//从CodeQueue收取
		iRet = m_stCodeQueueMgr.RecvOneMsg(pszMsg, iMaxOutMsgLen, riMsgLength, m_iRecvBusIndex);
		stFromBusID.usServerID = GAME_SERVER_LOTUS;
		stFromBusID.usInstanceID = m_iRecvBusIndex;
	}
	else if(m_iRecvBusIndex < (iCodeQueueNum +m_iBusNum))
	{
		//从ZMQ BUS收取
		iRet = m_astServerBus[m_iRecvBusIndex-iCodeQueueNum].ZmqRecv(pszMsg, iMaxOutMsgLen, riMsgLength, 0);
		GetBusStructID(m_astServerBus[m_iRecvBusIndex - iCodeQueueNum].GetBusID(), stFromBusID);
	}
	else
	{
		LOGERROR("Failed to recv msg from server bus, index %d, codequeue num %d, zmqbus num %d\n", m_iRecvBusIndex, iCodeQueueNum, m_iBusNum);
		iRet = -1;
	}

	if (usMsgPeer == GAME_SERVER_LOTUS)
	{
		//指定接收CodeQueue
		m_iRecvBusIndex = (m_iRecvBusIndex + 1) % iCodeQueueNum;
	}
	else if (usMsgPeer != GAME_SERVER_UNDEFINE)
	{
		//指定接收ZMQ BUS
		m_iRecvBusIndex = iCodeQueueNum + (m_iRecvBusIndex + 1) % m_iBusNum;
	}
	else
	{
		m_iRecvBusIndex = (m_iRecvBusIndex + 1) % (iCodeQueueNum + m_iBusNum);
	}

	return iRet;
}

//获取ZMQ BUS
ZmqBus* CServerBusManager::GetZmqBus(uint64_t ullBusID)
{
	for (int i = 0; i < m_iBusNum; ++i)
	{
		if (m_astServerBus[i].GetBusID() == ullBusID)
		{
			return &m_astServerBus[i];
		}
	}

	return NULL;
}
