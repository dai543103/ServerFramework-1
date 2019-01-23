#pragma once

#include "ZmqBus.hpp"
#include "CodeQueueManager.hpp"
#include "CommDefine.h"
#include "ServerBusConfigManager.h"

using namespace ServerLib;

const int MAX_SERVER_BUS_NUM = 8;

class CServerBusManager
{
public:
	static CServerBusManager* Instance();
	~CServerBusManager();

public:
	//初始化通信BUS
	int Init(const char* pszServerName, ServerBusConfigManager& stBusConfigMgr);

	//发送消息
	int SendOneMsg(const char* pszMsg, int iMsgLength, const SERVERBUSID& stToBusID);

	//接收消息
	int RecvOneMsg(char* pszMsg, int iMaxOutMsgLen, int& riMsgLength, SERVERBUSID& stFromBusID);

private:
	CServerBusManager();

	//获取ZMQ BUS
	ZmqBus* GetZmqBus(uint64_t ullBusID);

private:
	//轮询收消息，当前收消息的BusIndex
	int m_iRecvBusIndex;

	//与Lotus用CodeQueue通信
	CCodeQueueManager m_stCodeQueueMgr;

	//内网用ZMQBUS通信
	int m_iBusNum;
	ZmqBus m_astServerBus[MAX_SERVER_BUS_NUM];
};
