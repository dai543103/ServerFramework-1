#pragma once

#include "MsgHandlerSet.hpp"
#include "AddRegAuthHandler.hpp"
#include "DeleteRegAuthHandler.hpp"
#include "FetchRegAuthHandler.hpp"
#include "AuthAccountHandler.h"

// RegAuthDB应用中的消息处理者管理器
class CRegAuthDBHandlerSet : public CMsgHandlerSet
{
public:
    CRegAuthDBHandlerSet();
    ~CRegAuthDBHandlerSet();

	//初始化消息处理管理器
	virtual int Initialize(int iThreadIndex = -1);

private:
	int RegisterAllHandlers();

private:
	// 该集合管理的所有消息处理者
	CAddRegAuthHandler m_stAddRegAuthHandler;
	CDeleteRegAuthHandler m_stDeleteRegAuthHandler;
	CFetchRegAuthHandler m_stFetchRegAuthHandler;
	CAuthAccountHandler m_stAuthAccountHandler;

private:
	// 消息处理者处理消息时需要访问数据库
	DBClientWrapper m_oDBClient;

	//DB操作缓冲区
	char* m_pQueryBuff;

	int m_iThreadIdx;
};
