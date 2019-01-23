#pragma once

#include "MsgHandlerSet.hpp"
#include "WriteLogHandler.hpp"

// LogServer应用中的消息处理者管理器
class CLogServerHandlerSet : public CMsgHandlerSet
{
public:
    CLogServerHandlerSet();
    ~CLogServerHandlerSet();

    // 初始化该集合中的消息处理者和数据库配置并连接数据库
	virtual int Initialize(int iThreadIndex = -1);

private:
	int RegisterAllHandlers();

private:
	// 该集合管理的所有消息处理者
	CWriteLogHandler m_stWriteLogHandler;

private:
	// 消息处理者处理消息时需要访问数据库
	DBClientWrapper m_oDBClient;

	//DB操作缓冲区
	char* m_pQueryBuff;

	int m_iThreadIdx;
};
