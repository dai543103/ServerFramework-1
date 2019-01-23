
#include <string.h>
#include <arpa/inet.h>
#include "GameProtocol.hpp"
#include "LogAdapter.hpp"
#include "ServerBusManager.h"
#include "AppThreadManager.hpp"
#include "ExchangeProtocolEngine.hpp"

#include "ExchangeMsgProcessor.hpp"

using namespace ServerLib;

static GameProtocolMsg stMsg;

CExchangeMsgProcessor* CExchangeMsgProcessor::Instance()
{
	static CExchangeMsgProcessor* pInstance = NULL;
	if (!pInstance)
	{
		pInstance = new CExchangeMsgProcessor;
	}

	return pInstance;
}

CExchangeMsgProcessor::CExchangeMsgProcessor()
{
	//实际消息长度
	m_iCodeLen = 0;

	//发送缓冲区
	memset(m_szCodeBuff, 0, sizeof(m_szCodeBuff));
}

CExchangeMsgProcessor::~CExchangeMsgProcessor()
{

}

//初始化
int CExchangeMsgProcessor::Initialize()
{
	return 0;
}

//接收处理消息
void CExchangeMsgProcessor::ProcessRecvMsg(int& iNewMsgCount)
{
	iNewMsgCount = 0;
	m_iCodeLen = 0;

	// 接收网络数据
	SERVERBUSID stFromBusID;
	int iRet = CServerBusManager::Instance()->RecvOneMsg((char*)m_szCodeBuff, MAX_CODE_LEN, m_iCodeLen, stFromBusID);
	if ((iRet == 0) && (m_iCodeLen != 0))
	{
		LOGDEBUG("Receive net code OK, len: %d\n", m_iCodeLen);

		//发送到工作线程
		iRet = CAppThreadManager::Instance()->PollingPushCode(m_szCodeBuff, m_iCodeLen);
		if (iRet)
		{
			LOGERROR("Failed to send msg to work thread, ret %d\n", iRet);
		}
		else
		{
			//增加消息计数
			++iNewMsgCount;
		}
	}

	//接收平台线程数据
	for (int i = 0; i < WORK_THREAD_NUM; ++i)
	{
		m_iCodeLen = 0;
		iRet = CAppThreadManager::Instance()->PopCode(i, m_szCodeBuff, MAX_CODE_LEN, m_iCodeLen);
		if ((iRet == 0) && (m_iCodeLen != 0))
		{
			TRACESVR("Receive thread code OK, len: %d, thread index %d\n", m_iCodeLen, i);

			//发送到World
			SERVERBUSID stToBusID;
			stToBusID.usWorldID = 1;
			stToBusID.usServerID = GAME_SERVER_WORLD;
			stToBusID.usInstanceID = 1;

			iRet = CServerBusManager::Instance()->SendOneMsg((char*)m_szCodeBuff, m_iCodeLen, stToBusID);
			if (iRet)
			{
				//只打印错误日志，不返回
				LOGERROR("Failed to process thread msg, ret %d, from thread %d\n", iRet, i);
			}
			else
			{
				//增加消息计数
				++iNewMsgCount;
			}
		}
	}

	return;
}
