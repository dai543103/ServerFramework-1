#pragma once

#include "MsgHandlerSet.hpp"
#include "DBClientWrapper.hpp"
#include "OnlineExchangeHandler.hpp"
#include "GetCardNoHandler.hpp"

using namespace ServerLib;

// Exchange应用中的消息处理者管理器
class CExchangeHandlerSet : public CMsgHandlerSet
{
public:
	CExchangeHandlerSet();

	virtual ~CExchangeHandlerSet();

	//初始化消息处理管理器
	virtual int Initialize(int iThreadIndex = -1);

private:
	int RegisterAllHandlers();

private:

	//在线兑换消息
	COnlineExchangeHandler m_stOnlineExchangeHandler;

	//拉取卡密消息
	CGetCardNoHandler m_stGetCardNoHandler;

private:

	// 消息处理者处理消息时需要访问数据库
	DBClientWrapper m_oDBClient;

	//DB操作缓冲区
	char* m_pQueryBuff;

	//如果多线程，为线程index
	int m_iThreadIndex;
};
